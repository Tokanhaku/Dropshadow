/////////////////////////////////////////////////
/*
How to Compile:
1. brew install opencv
2. llvm-g++ `pkg-config --cflags --libs opencv4` -stdlib=libc++ -std=c++17 dropshadow.cpp -o dropshadow
*/
/////////////////////////////////////////////////

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat addShadow(Mat,int);
void overlayImage(Mat* src, Mat* overlay, const Point& location);
Mat addEdge(Mat*);

string getOutName(string inName){
    size_t last_dot = inName.rfind(".");
    return (inName.substr(0, last_dot) + " (dropshadow).png");
}

int main(int, char *argv[])
{
	Mat input;
	input = imread(argv[1], -1);
    string newName = getOutName(argv[1]);
    imwrite(newName, addShadow(input,75));
    
    /////////////////////////////////////////////
    /* Find the best gaussian filter radium
    /////////////////////////////////////////////
    long long int h,w,s;
    for (int k = 51; k < 82; k=k+2){
        Mat output = addShadow(input,k);
        Mat REF = imread("with.png", -1);
        h = input.rows;
        w = input.cols;
        s = 0;
     
        for (int i = 0; i < h; ++i){
            for (int j = 0; j < w; ++j){
                s += (int(output.at<cv::Vec4b>(i,j).val[3])-int(REF.at<cv::Vec4b>(i,j).val[3]))*(int(output.at<cv::Vec4b>(i,j).val[3])-int(REF.at<cv::Vec4b>(i,j).val[3]));
            }
        }
        cout << "k= " << k << " : " << s << endl;
    }
    */
    /////////////////////////////////////////////

	return 0;
}

Mat addShadow(Mat in_img, int r = 75){
    in_img = addEdge(&in_img);
    
    int h = in_img.rows;
    int w = in_img.cols;
    int c = in_img.channels();

    // 创建阴影
    cv::Mat shadow;
	in_img.copyTo(shadow);
    for (int y = 0; y < h; ++y){
    	for (int x = 0; x < w; ++x){
    		auto p = shadow.at<cv::Vec4b>(y,x).val;
    		for (int i = 0; i < 3; ++i){
    			p[i] = 0;
    		}
    		p[3] = int(p[3]*0.5);
    	}
    }
    int margin = 111;
    int offset = -36;
    int new_w = margin*2 + w;
    int new_h = margin*2 + h;

    cv::Mat out_img(new_h, new_w, CV_8UC4, cv::Scalar(0,0,0,0));
    shadow.copyTo(out_img(cv::Rect(margin, margin, w, h)));

    // 对阴影进行高斯模糊
    for ( int i = 1; i < r; i = i + 2 ){
        GaussianBlur( out_img, out_img, Size( i, i ), 0, 0 );
    }

    overlayImage(&out_img, &in_img, Point(margin,margin+offset));
    return out_img;
}

void overlayImage(Mat* src, Mat* overlay, const Point& location){
    for (int y = max(location.y, 0); y < src->rows; ++y)
    {
        int fY = y - location.y;

        if (fY >= overlay->rows)
            break;

        for (int x = max(location.x, 0); x < src->cols; ++x)
        {
            int fX = x - location.x;

            if (fX >= overlay->cols)
                break;

            double opacity = ((double)overlay->data[fY * overlay->step + fX * overlay->channels() + 3]) / 255;

            for (int c = 0; opacity > 0 && c < src->channels(); ++c)
            {
                unsigned char overlayPx = overlay->data[fY * overlay->step + fX * overlay->channels() + c];
                unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
                src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
            }
        }
    }
}

Mat addEdge(Mat* input){
    int h = input->rows;
    int w = input->cols;
    int c = input->channels();
    int edge_value = 120;
    Mat output(h+2, w+2, CV_8UC4, cv::Scalar(0,0,0,0));
    input->copyTo(output(cv::Rect(1, 1, w, h)));

    auto px = output.at<cv::Vec4b>(0,0).val;
    bool is_edge = false;

    for (int i = 0; i < h; ++i){
        for (int j = 0; j < w; ++j){
            is_edge = false;
            // cout << int(output.at<cv::Vec4b>(i,j).val[3]) << endl;
            if (int(input->at<cv::Vec4b>(i,j).val[3]) > edge_value){
                for (int u = -1; u < 2; ++u){
                    for (int v = -1; v < 2; ++v){
                        if (u==0 && v==0) break;
                        else if (output.at<cv::Vec4b>(i+1+v,j+1+u).val[3] < edge_value){
                            output.at<cv::Vec4b>(i+1+v,j+1+u).val[3] = edge_value;
                        }
                    }
                }
            }
        }
    }
    return output;
}

