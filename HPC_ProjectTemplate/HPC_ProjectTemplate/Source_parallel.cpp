#include <stdio.h>
#include <math.h>
#include <iostream>
#include<string.h>
#include <algorithm>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#include <mpi.h>
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
	int* Red = new int[BM.Height * BM.Width];
	int* Green = new int[BM.Height * BM.Width];
	int* Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

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
			if (image[i * width + j] < 0)
			{
				image[i * width + j] = 0;
			}
			if (image[i * width + j] > 255)
			{
				image[i * width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("E://FCIS//6th semster//HPC//Project//Output//" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	MPI_Init(NULL, NULL);
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	string path = "E://FCIS//6th semster//HPC//Project//BackGround//in000";//path of the images 
	int ImageWidth = 5, ImageHeight = 5;
	int indx = 0;
	int imgscnt=495;
	int pixels = ImageWidth * ImageHeight;
	int imgsperrank = imgscnt / size;

	if (rank == 0)
	{
		string s = "E://FCIS//6th semster//HPC//Project//BackGround//in000001.jpg";
		int* imageData = inputImage(&ImageWidth, &ImageHeight, marshal_as<System::String^>(s));
		pixels = ImageHeight * ImageWidth;

	}
	MPI_Bcast(&pixels, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int* imgs = new int[pixels * imgscnt];
	if (rank == 0)
	{
		//rading the images and filling the vector with them 
		for (int i=1;i<=imgscnt;i++)
		{
			string imagePath =to_string(i);
			string dest = string(3 - imagePath.size(), '0').append(imagePath);
			imagePath=path+dest+".jpg";
			int* imageData = inputImage(&ImageWidth, &ImageHeight, marshal_as<System::String^>(imagePath));
			pixels = ImageWidth * ImageHeight;
			for (int j = 0; j < pixels; j++)
			{
				imgs[(i-1) * pixels + j] = imageData[j];
			}
		
		}

	}
	MPI_Bcast(imgs,imgscnt*pixels, MPI_INT, 0, MPI_COMM_WORLD);

	int* Background = new int[pixels]();
	int* Foreground = new int[pixels]();
	int* localimg = new int[pixels / size]();
	int* localFg = new int[pixels/size]() ;
	int* localBg = new int[pixels / size]();
	int* img = new int[pixels]();
	
	int thrsh = 50;
	int n = pixels / size;
	int start = rank * n;
	int	end = start+n;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < imgscnt; j++)
		{
			localBg[i] += imgs[j*pixels+start+i];
		}

		localBg[i] /= imgscnt;
	}
	MPI_Gather(localBg, pixels / size, MPI_INT, Background, pixels / size, MPI_INT, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		createImage(Background, ImageWidth, ImageHeight, 100);
	}
	MPI_Bcast(Background, pixels, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		img = inputImage(&ImageWidth, &ImageHeight, "E://FCIS//6th semster//HPC//Project//BackGround//in000149.jpg");
	}
	MPI_Bcast(img, pixels, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(img, pixels / size, MPI_INT, localimg, pixels / size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(Background, pixels / size, MPI_INT, localBg, pixels / size, MPI_INT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < n; i++)
	{
		int isFg = abs(localBg[i] - localimg[i]);
		if (isFg > thrsh)
			localFg[i] = isFg;
	}
	MPI_Gather(localFg,pixels/size,MPI_INT,Foreground,pixels/size,MPI_INT,0,MPI_COMM_WORLD);
	if (rank == 0)
	{
		createImage(Foreground, ImageWidth, ImageHeight, 500);
	}

	MPI_Finalize();

	return 0;

}
