#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <cstdlib>
#include "cimage.h"
#include "bmplib.h"

using namespace std;
void doInputLoop(std::istream& istr, CImage& img); // prototype

int main(int argc, char *argv[])
{
    if(argc < 2) {
        cout << "Usage ./compedit in_img_filename <option option-args>" << endl;
        return 1;
    }
    CImage img1(argv[1]);//read image!
    img1.findComponents();

    int option = 0;
    if(argc >= 3) {
        option = atoi(argv[2]);
    }
    if(option == 1) {
        img1.printComponents();
        return 0;
    }
    else if(option == 2 || option == 3) {
        if(argc < 4) {
            cout << "Please provide an extra argument of the filename to save the image." << endl;
        }
        else if(option == 2){
            img1.labelToRGB(argv[3]);
        }
        else if(option == 3){
            img1.drawBoundingBoxesAndSave(argv[3]);
        }
        return 0;
    }

		// ===================
		// Setup input stream
		if(option == 4) {
        if(argc < 4) {
            cout << "Please provide an extra argument of the command filename." << endl;
            return 0;
        }        
        string cmd_error = "Bad command filename";

        // ===================
        // input stream. Print the string cmd_error and return 0
        // if the file can't be opened
        ifstream ifile(argv[3]);//continue

        if(ifile.fail()){
            cout << cmd_error << endl;
            return 0;
        }
        // ===================
        //  Pass the input stream created
        doInputLoop(ifile, img1); 
        // ===================
        // Close the file
        ifile.close();
		}
    else {
        doInputLoop(cin, img1);
    }

    
    return 0;
}

void doInputLoop(std::istream& istr, CImage& img)
{
    // ===================
    // process user commands
    //  of t - translate, f - forward, b - backward, s - save, and
    //  any other input letter to quit.
    string option_error = "Bad option";
    char option = 'q';
    int cid;
    bool again = true;
    do {
        img.printComponents();
        cout << "\nEnter a command [t,f,b,s,q]: " << endl;
        istr >> option;
        if(option == 't') {
            int nr, nc;
            // ==================
            // Read cid, nr, and nc from the input stream,
            //       printing "Bad option" and breaking from the 
            //       loop if any of the input is not read successfully
            istr>>cid>>nr>>nc;
           
            img.translate(cid, nr, nc);
        }
        else if(option == 'f' || option == 'b') {
            int delta;
            // ==================
            // Read cid and delta from the input stream
            //       printing "Bad option" and breaking from the 
            //       loop if any of the input is not read successfully
            istr>>cid>>delta;
            if(option == 'f') {
                img.forward(cid, delta);
            }
            else {
                img.backward(cid, delta);
            }
        }
        else if(option == 's') {
            string filename;
            
            istr>>filename;
            // ==================
            //  Read filename from the input stream,
            //       printing "Bad option" and breaking from the 
            //       loop if any of the input is not read successfully
            
            img.save(filename.c_str());
        }
        else if(option=='q'){
            again = false;
        }
        else
        {
          cout<<"Bad option"<<endl;
          again = false;
        }
    } while(again);
}
