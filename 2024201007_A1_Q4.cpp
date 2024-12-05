//g++ 2024201007_A1_Q4.cpp -o Q4 `pkg-config --cflags --libs opencv4`
#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

int compute_energy(const cv::Mat& image, int y, int x) {
    int height = image.rows;
    int width = image.cols;	
    int left,right,up,down;
    if(x==0) left = width-1;
    else left = x-1;
    if(x==width-1) right =0;
    else right = x+1;
    if(y==0) up = height-1;
    else up = y-1;
    if(y==height-1) down=0;
    else down = y+1;
    // row,column
    cv::Vec3b color_left = image.at<cv::Vec3b>(y, left);
    cv::Vec3b color_right = image.at<cv::Vec3b>(y, right);
    cv::Vec3b color_up = image.at<cv::Vec3b>(up, x);
    cv::Vec3b color_down = image.at<cv::Vec3b>(down, x);
    //cout<<color_right<<" "<<color_left<<" "<<color_up<<" "<<color_down<<"\n";
    //[133, 122, 64] [134, 126, 79] [124, 116, 69] [141, 129, 71]
    int sX = 0,sY=0;
    //B, G, R channels
    for (int c = 0; c < 3; c++){ 
        int dX = color_right[c] - color_left[c];
        int dY = color_down[c] - color_up[c];
        sX += dX*dX;
        sY += dY*dY;
    }
    return sqrt(sX + sY);
}

cv::Mat remove_vertical_seam(cv::Mat image, bool visualize) {
    int height = image.rows;
    int width = image.cols;

    int** energy = new int*[height];

    for (int i = 0; i < height; ++i) {
        energy[i] = new int[width];
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            energy[i][j] = compute_energy(image, i, j);
        }
    }

    int** dp = new int*[height];
    for (int i = 0; i < height; ++i) {
        dp[i] = new int[width];
    }

    for (int j = 0; j < width; j++) {
        dp[0][j] = energy[0][j];
    }

    for (int i = 1; i < height;i++) {
        for (int j = 0; j < width; j++) {
            int mini = dp[i - 1][j];
            if(i>0 && j>0) mini = min(mini, dp[i - 1][j - 1]);
            if(i>0 &&j<width-1) mini = min(mini, dp[i - 1][j + 1]);
            dp[i][j] = energy[i][j] + mini;
        }
    }

    int* seam = new int[height];
    int min_y = 0;
    for (int j= 1; j < width; j++) {
        if (dp[height-1][j] < dp[height-1][min_y]) {
            min_y = j;
        }
    }
    seam[height - 1] = min_y;
    for(int i=height-2; i>=0;i--) {
        min_y = seam[i+ 1];
        if(min_y > 0 && dp[i][min_y - 1] < dp[i][min_y]){
            min_y--;
        } else if(min_y < width-1 && dp[i][min_y+1] < dp[i][min_y]) {
            min_y++;
        }
        seam[i] = min_y;
    }

    if (visualize) {
        for (int i = 0; i < height; i++) {
            image.at<cv::Vec3b>(i, seam[i]) = cv::Vec3b(0, 0, 255); // Set seam pixel to red (BGR)
        }
        cv::imshow("", image);
        cv::waitKey(90); 
    }

    cv::Mat output_image(height, width - 1, CV_8UC3);
    for (int i = 0; i < height; i++) {
        int seam_j = seam[i];
        for (int j = 0, new_j = 0; j < width; j++) {
            if (j == seam_j) continue;
            output_image.at<cv::Vec3b>(i, new_j++) = image.at<cv::Vec3b>(i, j);
        }
    }

    for (int i = 0; i < height; ++i) {
        delete[] energy[i];
        delete[] dp[i];
    }
    delete[] energy;
    delete[] dp;
    delete[] seam;

    return output_image;
}

cv::Mat remove_horizontal_seam(cv::Mat image, bool visualize) {
    cv::transpose(image, image);
    image = remove_vertical_seam(image, visualize);
    cv::transpose(image, image);
    return image;
}

int main() {
    cout << "Enter the path of the input image: ";
    string filename;
    cin >> filename;
    int new_width, new_height;
    cout << "\nEnter new width and height: ";
    cin >> new_width >> new_height;
    
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        cout << "Could not open or find the image\n";
        return -1;
    }

    int height = image.rows;
    int width = image.cols;
    if(new_width>width || new_height>height){
        cout << "The dimensions of the new image must be smaller!\n";
        return -1;
    }
    while (width > new_width) {
        image = remove_vertical_seam(image, true); 
        width = image.cols;
    }

    while (height > new_height) {
        image = remove_horizontal_seam(image, true); 
        height = image.rows;
    }
    filename = filename + "_resized";
    cv::imwrite(filename, image);
    string output = "Output resized image produced as: " + filename;
    cout << output << endl;

    return 0;
}

