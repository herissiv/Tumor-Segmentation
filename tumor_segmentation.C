#include <TCanvas.h>
#include <TASImage.h>


//Comment in/out which image you want to look/not look at:

//Lung nr 1: 
const char* pic1 = "lung_tumor.jpg";
const char* mod1 = "modified_lung_tumor.jpg";
const char* pre1 = "preproc_lung_tumor.jpg";
const char* seg1 = "seg_lung_tumor.jpg";

//Lung nr. 2:
// const char* pic1 = "tumor2.jpg";
// const char* mod1 = "modified_tumor2.jpg";
// const char* pre1 = "preproc_tumor2.jpg";
// const char* seg1 = "seg_tumor2.jpg";

//Lung nr. 3:
// const char* pic1 = "tumor3.jpg";
// const char* mod1 = "modified_tumor3.jpg";
// const char* pre1 = "preproc_tumor3.jpg";
// const char* seg1 = "seg_tumor3.jpg";


//Simple functions to get the width and heigth of the images
int get_width(const char* jpg){
    TASImage image(jpg);
    UInt_t width  = image.GetWidth();
    return width;
}

int get_height(const char* jpg){
    TASImage image(jpg);
    UInt_t heigth  = image.GetHeight();
    return heigth;
}


int height = get_height(pic1);
int width = get_width(pic1);
int K;

//Convert from jpg to a TASImage
TASImage generate_TAS(const char* jpg){
    TASImage image(jpg);
    return image;
}

//Function to open a jpg file
void open_jpg(const char* jpg) {
    TImage* image = TImage::Open(jpg);

    if (!image) {
        std::cout << "Failed to open the image file." << std::endl;
        return;
    }

    TCanvas* canvas = new TCanvas("canvas", "Image Display");
    canvas->cd();
    image->Draw();
    canvas->Update();

    canvas->WaitPrimitive();

    delete image;
}

//Function to read the TASImage, and extract the greyscale value from the specified position
int read_TAS(TASImage& image, int row, int col){
    UInt_t *argb = image.GetArgbArray(); 
    int grey;
    K = row * width + col;

    // Extract grayscale value
    grey = argb[K] & 0xFF;  
    return grey;
}



//Function for changing a pixelvalue (greyscale) to a specified greyscale value. 
void change_pixel(TASImage& image, int row, int col, int grey){
    UInt_t *argb = image.GetArgbArray(); 
    if (row < 0 || row > height || col < 0 || col > width){
        cout << "The pixel with row: " << row << " col: " << col << " dosent exists" << endl;
    }
    K = row * width + col;
    argb[K] = (argb[K] & 0xFF000000) | (static_cast<UInt_t>(grey) << 16) |
        (static_cast<UInt_t>(grey) << 8) | static_cast<UInt_t>(grey);
}

//Functuion similar for the function above, but changes the color to red 
void change_red_pixel(TASImage& image, int row, int col){
    UInt_t *argb = image.GetArgbArray(); 
    if (row < 0 || row > height || col < 0 || col > width){
        cout << "The pixel dosent exists" << endl;
        exit(-1);
    }
    K = row * width + col;
    argb[K] = (argb[K] & 0xFF000000) | (255 << 16);
}

//Preprocessing function for making the TASImage black and white, given a blackness limit.
void preprocessing(TASImage& image, int blacklimit){
    for (int row = 0; row<height; row++){
        for (int col = 0; col<width; col++){
            if (read_TAS(image,row,col) <= blacklimit){
                change_pixel(image, row, col, 0);
            } 
            else{
                change_pixel(image, row, col, 255);
            }
        }
    }

    //Store the preprocessed image 
    std::string preprocessed = "preproc_" + std::string(pic1);  
    image.WriteImage(preprocessed.c_str());
    std::cout << "Image modified and saved as " << preprocessed << std::endl;
}

//Segmentation function for painting pixels red when the loop meets on structures in the lung itself. 
void segmentation(TASImage& image, TASImage& final_image, int width_included, int height_included){
    
    // height/2 will be the row in the middle of the image, 
    // ignores structures on the top of the lung and under the lung.
    // height_included tells us how much from the middle we want to look at, 
    // and height_included should be greater or equal to 2. 
    for (int row = height/2 - height/height_included; row<height/2 + height/height_included; row++){
        
        // width/2 will have the same effect as height/2.
        // After set the correct value for width_included and height_included, 
        // the segmentation should just take inside the lung itself.
        for (int col = width/2-width/width_included; col<width/2 +width/width_included; col++){
            if (read_TAS(image,row,col) == 0 && read_TAS(image,row+1,col+1) == 255 || read_TAS(image,row,col) == 255 && read_TAS(image,row+1,col+1) == 0){
                
                //For ignoring the middle part of the lung 
                if (col <width/2-width/(3*width_included) || col>width/2+width/(3*width_included)){
                    if (read_TAS(image,row,col) == 0){
                        for (int r = row-3; r<=row; r++){
                            for (int c = col-3; c<=col; c++){
                                change_red_pixel(final_image, r, c);
                            }
                        }
                    }
                    if (read_TAS(image,row+1,col+1) == 0 && row+1 < height){
                        for (int r = row+1; r<=row+3; r++){
                            for (int c = col+1; c<=col+3; c++){
                                change_red_pixel(final_image, r, c);
                            }
                        }
                    }
                }
            } 
        }
    }

    //Store the segmented image 
    std::string seg = "seg_" + std::string(pic1);  
    final_image.WriteImage(seg.c_str());
    std::cout << "Image modified and saved as " << seg << std::endl;
}


void tumor_segmentation(){
    
    cout << "Creating TASImage" << endl;
    TASImage TASpic1 = generate_TAS(pic1);
    cout << "Preprocessing" << endl;
    preprocessing(TASpic1,120);
    

    cout << "Creating TASImage" << endl;
    TASImage TASseg1 = generate_TAS(pre1);

    cout << "segmentating" << endl;
    TASImage TASpic = generate_TAS(pic1);
    segmentation(TASseg1, TASpic, 5, 5);
    cout << "Opening the segmented image" << endl;
    open_jpg(seg1);

}

int main() {
    tumor_segmentation();
    return 0;
}

