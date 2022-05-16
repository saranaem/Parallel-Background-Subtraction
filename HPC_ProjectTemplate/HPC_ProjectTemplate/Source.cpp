#include <stdio.h>
#include <math.h>
#include <iostream>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;
#include <filesystem>
namespace fs = std::filesystem;
int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height*BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("E://FCIS//6th semster//HPC//Project//Output//" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	cout << "Yarab" << endl;
	string path = "E://FCIS//6th semster//HPC//Project//BackGround";//path of the images 
	vector<string>img_paths;
	vector<int*>images;///vector of image streams 

	int ImageWidth = 5, ImageHeight = 5;
	int indx = 0;

	//rading the images and filling the vector with them 
	for (const auto& entry : fs::directory_iterator(path))
	{
		string temp = entry.path().string();
		System::String^ imagePath;
		imagePath = marshal_as<System::String^>(temp);

		int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
		images.push_back(imageData);
	}
	int imgscnt;
	int pixels = ImageWidth * ImageHeight;
	int* Background = new int[pixels]();
	int* Foreground = new int[pixels]();
	imgscnt = images.size();
	for (int i = 0; i < pixels; i++)
	{

		for (int j = 0; j < imgscnt; j++)
		{
			Background[i] += images[j][i];
		}
		Background[i] /= imgscnt;
	}
	createImage(Background, ImageWidth, ImageHeight, 0);
	int* img=inputImage(&ImageWidth, &ImageHeight, "E://FCIS//6th semster//HPC//Project//BackGround//in000149.jpg");
	int thrsh = 50;
	for (int i = 0; i < pixels; i++)
	{
		int isFg = abs(Background[i] - img[i]);
		if(isFg>thrsh)
			Foreground[i] =isFg ;
	}
	createImage(Foreground, ImageWidth, ImageHeight, 3);
	return 0;

}



