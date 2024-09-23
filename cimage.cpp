#include "component.h"
#include "cimage.h"
#include "bmplib.h"
#include <deque>
#include <iomanip>
#include <iostream>
#include <cmath>

using namespace std;


CImage::CImage(const char* bmp_filename)
{
    //  Note: readRGBBMP dynamically allocates a 3D array
    //    (i.e. array of pointers to pointers (1 per row/height) where each
    //    point to an array of pointers (1 per col/width) where each
    //    point to an array of 3 unsigned char (uint8_t) pixels [R,G,B values])

    // ================================================
    // call readRGBBMP to initialize img_, h_, and w_;

    img_= readRGBBMP(bmp_filename, h_, w_);//?>???


    if(img_ == NULL) {
        throw std::logic_error("Could not read input file");
    }

    // Set the background RGB color using the upper-left pixel
    for(int i=0; i < 3; i++) {
        bgColor_[i] = img_[0][0][i];
    }

    // RGB "distance" threshold to continue a BFS from neighboring pixels
    bfsBgrdThresh_ = 60;

    // ================================================
    // Initialize the vector of vectors of labels to -1
    for(int i=0; i < h_; i++){
    // Allocate an inner vector of m entries for each row
    vector<int> myrow(w_, -1);
    labels_.push_back(myrow);
      }

    
}

CImage::~CImage()
{
  
  deallocateImage(img_);
}

bool CImage::isCloseToBground(uint8_t p1[3], double within) {
    // Computes "RGB" (3D Cartesian distance)
    double dist = sqrt( pow(p1[0]-bgColor_[0],2) +
                        pow(p1[1]-bgColor_[1],2) +
                        pow(p1[2]-bgColor_[2],2) );
    return dist <= within;
}


size_t CImage::findComponents()
{
  size_t cnt=0;
    //bfsComponent(int pr, int pc, int mylabel)
    for(int i=0; i < h_; i++) {
        for (int j=0; j<w_; j++)
        {
          if(!isCloseToBground(img_[i][j], bfsBgrdThresh_)&&labels_[i][j]==-1)//if not close and not visited
          {
            components_.push_back(bfsComponent(i, j, cnt++));
          }
        }
    }

  return cnt-1;
}

void CImage::printComponents() const
{
    cout << "Height and width of image: " << h_ << "," << w_ << endl;
    cout << setw(4) << "Ord" << setw(4) << "Lbl" << setw(6) << "ULRow" << setw(6) << "ULCol" << setw(4) << "Ht." << setw(4) << "Wi." << endl;
    for(size_t i = 0; i < components_.size(); i++) {
        const Component& c = components_[i];
        cout << setw(4) << i << setw(4) << c.label << setw(6) << c.ulNew.row << setw(6) << c.ulNew.col
             << setw(4) << c.height << setw(4) << c.width << endl;
    }

}


int CImage::getComponentIndex(int mylabel) const
{
  for (int i=0; i<numComponents(); i++)
  {
    if(components_[i].label==mylabel)
    {
      return i;
    }
  }
return 0;
}



//   Add checks to ensure the new location still keeps
//   the entire component in the legal image boundaries
void CImage::translate(int mylabel, int nr, int nc)//bug
{
    // Get the index of the specified component
    int cid = getComponentIndex(mylabel);
    if(cid < 0) {
        return;
    }
    int h = components_[cid].height;
    int w = components_[cid].width;

    // ==========================================================
    // CHECK IF THE COMPONENT WILL STILL BE IN BOUNDS
    // IF NOT:  JUST RETURN.
    if (!(nr+h<=h_&&nc+w<=w_&&nr>=0&&nc>=0))
    {
      return;
    }
    // ==========================================================

    // If we reach here we assume the component will still be in bounds
    // so we update its location.
    Location nl(nr, nc);
    //cout<<"nr"<<
    components_[cid].ulNew = nl;
}


void CImage::forward(int mylabel, int delta)
{
    int cid = getComponentIndex(mylabel);
    if(cid < 0 || delta <= 0) {
        return;
    }
    int change =cid+delta;
    if (change>=numComponents())
    {
      change=numComponents()-1;
    }
    Component c = components_[cid];
    for (int i=cid; i<change; i++)
    {
      components_[i] = components_[i+1];//0 1 2 3 //0 2 1 3 // 2 0 1 3
    }
    components_[change] = c;
    cout<<"moving it forward."<<endl;
    for (int i=0; i<numComponents(); i++)
    {
      cout<<components_[i].label<<" ";
    }

}

void CImage::backward(int mylabel, int delta)
{
    int cid = getComponentIndex(mylabel);
    if(cid < 0 || delta <= 0) {
        return;
    }
    int change =cid-delta;
    if (cid-delta<0)
    {
      change = 0;
    }
    
    Component c = components_[cid];
    cout<<"about to run for loop."<<endl;
  for (int i=cid; i>change; i--)
    {
      components_[i] = components_[i-1];
    }
    components_[change] = c;
}

void CImage::save(const char* filename)
{
    // Create another image filled in with the background color
    uint8_t*** out = newImage(bgColor_);
   
      for (int start=0; start<numComponents(); start++)
      {
        int tempX=components_[start].ulOrig.row;
        
          for (int h=components_[start].ulNew.row; h<components_[start].height+components_[start].ulNew.row; h++)//height and width of bounding box
          {
            int tempY=components_[start].ulOrig.col;

            for (int w=components_[start].ulNew.col; w<components_[start].width+components_[start].ulNew.col; w++)
            {
              if (labels_[tempX][tempY]==components_[start].label)
              {
                for (int i=0; i<3; i++)
                {
                  out[h][w][i]=img_[tempX][tempY][i];
                }
              }
              tempY++;
            }
            tempX++;
          }
      }
    
    writeRGBBMP(filename, out, h_, w_);
}

//Creates a blank image with the background color
uint8_t*** CImage::newImage(uint8_t bground[3]) const
{
    uint8_t*** img = new uint8_t**[h_];
    for(int r=0; r < h_; r++) {
        img[r] = new uint8_t*[w_];
        for(int c = 0; c < w_; c++) {
            img[r][c] = new uint8_t[3];
            img[r][c][0] = bground[0];
            img[r][c][1] = bground[1];
            img[r][c][2] = bground[2];
        }
    }
    return img;
}


void CImage::deallocateImage(uint8_t*** img) const
{
  for(int i=0; i < h_; i++) {
        for (int j=0; j<w_; j++)
        {
          delete [] img[i][j];
        }
        delete [] img[i];
    }

  delete [] img;

}

// TODO: Complete the following function or delete this if
//       you do not wish to use it.
Component CImage::bfsComponent(int pr, int pc, int mylabel)
{
    // Arrays to help produce neighbors easily in a loop
    // by encoding the **change** to the current location.
    // Goes in order N, NW, W, SW, S, SE, E, NE
    int neighbor_row[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int neighbor_col[8] = {0, -1, -1, -1, 0, 1, 1, 1};
    //labels need to be updated eventually

    Location UL(pr, pc); 

    deque<Location> queue;
    queue.push_back(UL);
    int max_col=pc, max_row=pr, min_row=pr, min_col=pc;

    while(!queue.empty())
    {
      Location curr=queue.front();
      labels_[curr.row][curr.col]=mylabel;
      queue.pop_front();
      
      if(curr.row>max_row)
      {
          max_row=curr.row;
      }
      if(curr.row<min_row)
      {
          min_row=curr.row;
      }

      if (curr.col>max_col)
      {
        max_col=curr.col;
      }
      if (curr.col<min_col)
      {
        min_col=curr.col;
      }
      
      for (int i=0; i<8; i++)
      {
        if(curr.row+neighbor_row[i]>=0&&curr.col+neighbor_col[i]>=0&&curr.row+neighbor_row[i]<h_&&curr.col+neighbor_col[i]<w_)
        {
            if(labels_[curr.row+neighbor_row[i]][curr.col+neighbor_col[i]]==-1)
            {
              if (!isCloseToBground(img_[curr.row+neighbor_row[i]][curr.col+neighbor_col[i]],bfsBgrdThresh_))
              {
                labels_[curr.row+neighbor_row[i]][curr.col+neighbor_col[i]]=mylabel;
                queue.push_back(Location(curr.row+neighbor_row[i],curr.col+neighbor_col[i]));
              }
            }
        }
      }
    }
    UL.col=min_col;
    
    int w1=max_col-min_col+1;
    int h1=max_row-min_row+1;
    Component c(UL, h1, w1, mylabel);
    c.ulOrig.row = min_row;
  c.ulOrig.col = min_col;
  c.ulNew.col = min_col;
  c.ulNew.row = min_row;
    
    return c;
}

//  Debugging function to save a new image
void CImage::labelToRGB(const char* filename)
{
    //multiple ways to do this -- this is one way
    vector<uint8_t[3]> colors(components_.size());
    for(unsigned int i=0; i<components_.size(); i++) {
        colors[i][0] = rand() % 256;
        colors[i][1] = rand() % 256;
        colors[i][2] = rand() % 256;
    }

    for(int i=0; i<h_; i++) {
        for(int j=0; j<w_; j++) {
            int mylabel = labels_[i][j];
            if(mylabel >= 0) {
                img_[i][j][0] =  colors[mylabel][0];
                img_[i][j][1] =  colors[mylabel][1];
                img_[i][j][2] =  colors[mylabel][2];
            } else {
                img_[i][j][0] = 0;
                img_[i][j][1] = 0;
                img_[i][j][2] = 0;
            }
        }
    }
    writeRGBBMP(filename, img_, h_, w_);
}

const Component& CImage::getComponent(size_t i) const
{
    if(i >= components_.size()) {
        throw std::out_of_range("Index to getComponent is out of range");
    }
    return components_[i];
}

size_t CImage::numComponents() const
{
    return components_.size();
}

void CImage::drawBoundingBoxesAndSave(const char* filename)
{
    for(size_t i=0; i < components_.size(); i++){
        Location ul = components_[i].ulOrig;
        int h = components_[i].height;
        int w = components_[i].width;
        for(int i = ul.row; i < ul.row + h; i++){
            for(int k = 0; k < 3; k++){
                img_[i][ul.col][k] = 255-bgColor_[k];
                img_[i][ul.col+w-1][k] = 255-bgColor_[k];

            }
            // cout << "bb " << i << " " << ul.col << " and " << i << " " << ul.col+w-1 << endl; 
        }
        for(int j = ul.col; j < ul.col + w ; j++){
            for(int k = 0; k < 3; k++){
                img_[ul.row][j][k] = 255-bgColor_[k];
                img_[ul.row+h-1][j][k] = 255-bgColor_[k];
            }
            // cout << "bb2 " << ul.row << " " << j << " and " << ul.row+h-1 << " " << j << endl; 
        }
    }
    writeRGBBMP(filename, img_, h_, w_);
}



