#include <opencv2/opencv.hpp>
#include<windows.h>
#include<stdlib.h>
#include<iostream>
#include<stdio.h>
#include<math.h>
#include<string>
#include<vector>
#include<tchar.h>
#include<conio.h>
#include<complex>
using namespace std;
using namespace cv;

/***************GLOBAL DECLARATION*****************************/
const int Smax = 24;
 int position;
 
 Mat InputFrame,YF,ycrcb[3],segmentedframe,smoothimage,boundaryoutput;
/****************FUNCTION DECLARATION***************************/
int otsu_threshold(Mat );
int fingerdetection(int ,int ,int ,int ,int ,int ,int ,Mat,int );
int segmentation(Mat ,Mat ,int );
int rowminimum(Mat);
int rowmaximum(Mat);
int colminimum(Mat);
int colmaxmum(Mat);
int orientation_detection(int ,int ,int ,int );
void boundary_matrix(Mat,int ,int ,int ,int );
int thumb_detection(Mat ,Mat ,int ,int ,int ,int ,int ,int );
int edge_detection(Mat ,Mat );
void centroid(Mat smoothimage,unsigned int *com_x,unsigned int *com_y);
void on_trackbar ( int,void *);
void classification (int orient ,int fingers,int tipcol[10],int tiprow[10], int distance,int thumb);
/******************************************MAIN FUNCTION************************/
void main (int argc, char *argv[])
{
	position =0;   // position of trackbar set to 0

	// TRACKBAR WINDOW
     char* source_window = "Input";
     namedWindow( source_window, CV_WINDOW_AUTOSIZE );
	 createTrackbar( " trackbar: ", "Input", &position, Smax, on_trackbar);
	 on_trackbar(  position,0 );
	
	waitKey(0);
	getch();
}

/************************************* OTHER FUNCTIONS **********************************/
int otsu_threshold(Mat InputFrame)
{
	/*----------histogram of image-----*/
	int rows,cols;
	rows=InputFrame.rows;
	cols=InputFrame.cols;
	int histogram[256];
	int i,j;
	for(i=0;i<256;i++){
          histogram[i]=0;}
	for(i=0;i<rows;i++){
	for(j=0;j<cols;j++){
		int temp;
		temp=(int)InputFrame.at<unsigned char>(i, j);
	    histogram[temp]=histogram[temp]+1;
	}
}

int sum = 0;
    for (i = 1; i < 256; ++i)
        sum =sum+ i * histogram[i];  // for '0', no change
    float sumB ;
    float wB ;
    float wF ;
    float mB;
    float mF;
    float max ;
    float bet ;
    float threshold1 ;
    float threshold2 ;
	  sumB = 0;
     wB = 0;
     wF = 0;
	  max = 0;
    bet = 0;
    threshold1 = 0;
    threshold2 = 0;
    for (i = 0; i < 256;i++) {
        wB =wB+histogram[i];
        if (wB == 0){
			continue;}
        wF = (rows*cols) - wB;
        if (wF == 0){
            break;}
        sumB=sumB+(i*histogram[i]);
        mB = sumB / wB;
        mF = (sum - sumB) / wF;
        bet = wB * wF * (mB - mF)*(mB - mF);
        if ( bet >= max ) {
            threshold1 = i;
            if ( bet > max ) {
            
				threshold2 = i;
            }
            max = bet;            
        }
    }
    return ( threshold1 + threshold2 ) / 2;
}

int fingerdetection(int flag,int colmin,int colmax,int rowmin,int rowmax,int com_x,int com_y,Mat edgeoutput,int thumb)
{
	// declaration
	static int barray[10000],carray[10000];
	int i=0;int flag1;
	int count=0;
	int peakrow[100],peakcol[100],h=0;
	double dist[30];
	double maximum;
	double threshold;
	int tiprow[10],tipcol[10],q=0;
	/*****************************************************************************************************************************/
	if(flag==1)
	{
	for (int col = colmin+1; col <= colmax+1;col++)
	{
		flag1=1;
		for (int row = rowmin+1; row <= com_x-10; row++)
		{
			unsigned char PixelValue = edgeoutput.at<unsigned char>(row, col);
						if (PixelValue==125 && flag1==1)
						{
							barray[i++] = row;
							i--;
							carray[i++] = col;
							flag1=0;
						}
		}
	}
	
	i--; 
	
	for(int j=1;j<i-3;j++)
	{
		if((barray[j-1]>=barray[j])&&(barray[j]<barray[j+1])&&(barray[j+1]<barray[j+2])&&(barray[j+2]<barray[j+3])&&(barray[j+3]<barray[j+4]))
		{
			peakrow[h]=barray[j];
			peakcol[h]=carray[j];
			h++;
			count++;
		}
	} 
	} /* end of if */

	/*****************************************************************************************************************************/

	
	if(flag==0)
	{
	
	
	for (int row = rowmin+1; row <= rowmax+1;row++)
	{
		flag1=1;
		for (int col = colmax+1; col >= com_y+7; col--)
		{
			unsigned char PixelValue = edgeoutput.at<unsigned char>(row, col);
						if (PixelValue==125 && flag1==1)
						{
							barray[i++] = col;
							i--;
							carray[i++] = row;
							flag1=0;
						}
		}
	} // end of for 
	
	i--;
	
	for(int j=1;j<i-3;j++)
	{
		
		if((barray[j-1]<=barray[j])&&(barray[j]>barray[j+1])&&(barray[j+1]>barray[j+2])&&(barray[j+2]>barray[j+3])&&(barray[j+3]>barray[j+4]))
		{
			peakcol[h]=barray[j];
			peakrow[h]=carray[j];
			h++;
			count++;
		}
	} // end of for
	} // end of if 
	/*****************************************************************************************************************************/

	printf("Total Peaks=%d\n",count);
	
	for(i=0;i<count;i++)
	{
		dist[i]=sqrt(float((peakrow[i]-com_x)*(peakrow[i]-com_x))+((peakcol[i]-com_y)*(peakcol[i]-com_y)));
		printf("PEAK%d=(%d,%d),Distance=%f\n",i+1,peakrow[i],peakcol[i],dist[i]);
		edgeoutput.at<unsigned char>(peakrow[i],peakcol[i])=255;
	}
	imshow("EDGE_output",edgeoutput);
	 maximum=dist[0];
	for(i=0;i<count;i++)
	{
		if(dist[i]>maximum)
			maximum=dist[i];
	}
	int f;
	if(maximum>115.0)
	{
		threshold=(0.75*maximum);
	
		printf("Maximum Distance=%f,Threshold distance=%f\n",maximum,threshold);
	
		for(i=0;i<count;i++)
		{
			if(dist[i]>threshold)
			{
				tiprow[q]=peakrow[i];
				tipcol[q]=peakcol[i];
				q++;
			}
		} // for closed 
		printf("SIGNIFICANT PEAKS ARE -->\n");
		for(i=0;i<q;i++)
		{
		printf("SIGNIFICANT PEAK%d=(%d,%d)\n",i+1,tiprow[i],tipcol[i]);
		} // for closed 

		printf("TOTAL FINGERS=%d\n",q);
	}
	
	else
	{ 
		q=0;
		printf("TOTAL NUMBER OF FINGER RAISED = 0");
		f=1;
		//thumb_detection(smoothimage,thumbbox,flag,pixelcount,rowmin,rowmax,colmin,colmax)
	}

	classification(flag,q,tipcol,tiprow,maximum,thumb);
	/************************************/
	
	return 0;
}

 int segmentation(Mat temp,Mat segmentedframe,int threshold)
 {
	int pixelcount=0;
		for ( int row = 0; row <temp.rows; row++)
			{
				for ( int col = 0; col <temp.cols; col++)

					{ 
						unsigned char PixelValue = temp.at<unsigned char>(row, col);
						if (PixelValue<(unsigned char)threshold)
						segmentedframe.at<unsigned char>(row, col)=0;
						else
						{
						segmentedframe.at<unsigned char>(row, col)=255;
						pixelcount++;
						}
			
					}
			}
		return (pixelcount);
 }

 int rowminimum(Mat smoothimage)
 {
	 int flag=1;
	 int r_min;
	 for ( int row = 0; row <smoothimage.rows; row++)
			{
				for ( int col = 0; col <smoothimage.cols; col++)

					{ 
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if (PixelValue==255 && flag==1)
						{
							r_min=row;
							flag=0;
						}
				}
	 }
	 return (r_min);
 }

int rowmaximum(Mat smoothimage)
{
	int flag=1;
	 int r_max;
	 for ( int row = smoothimage.rows-1; row >=0; row--)
			{
				for ( int col = 0; col <smoothimage.cols; col++)

					{ 
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if (PixelValue==255 && flag==1)
						{
							r_max=row;
							flag=0;
						}
				}
	 }
	 return (r_max);
}

int colminimum(Mat smoothimage)
{
	int flag=1;
	 int c_min;
	 for ( int col = 0; col <smoothimage.cols; col++)
			{
				for ( int row = 0; row <smoothimage.rows; row++)

					{ 
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if (PixelValue==255 && flag==1)
						{
							c_min=col;
							flag=0;
						}
				}
	 }
	 return (c_min);
}

int colmaxmum(Mat smoothimage)
{
	int flag=1;
	 int c_max;
	 for ( int col = smoothimage.cols-1; col >=0; col--)
			{
				for ( int row = 0; row <smoothimage.rows; row++)

					{ 
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if (PixelValue==255 && flag==1)
						{
							c_max=col;
							flag=0;
						}
				}
	 }
	 return (c_max);
}

int orientation_detection(int rowmin,int rowmax,int colmin,int colmax)
{
	int flag;
		int l=rowmax-rowmin;
		int w=colmax-colmin;
		if(l>w)
			flag=1;
		else
			flag=0;
		if(flag==1)
			printf("vertical hand\n");
		if(flag==0)
			printf("horizontal hand\n");
		return (flag);
}

void boundary_matrix(Mat boundaryoutput,int rowmin,int rowmax,int colmin,int colmax)
{
		for ( int row = rowmin; row <=rowmax; row++)
			{
				for ( int col = colmin; col <=colmax; col++)

					{ 
						if (row==rowmin||row==rowmax||col==colmin||col==colmax)
						boundaryoutput.at<unsigned char>(row, col)=125;
			
					}
			}
}

int thumb_detection(Mat smoothimage,Mat thumbbox,int flag,int pixelcount,int rowmin,int rowmax,int colmin,int colmax)
{
		int leftcount=0;
		int rightcount=0;
		int thumb =1;
		if(flag==0)
		{
			//horizontal
		for ( int row = rowmin; row <=rowmin+30; row++)
			{
				for ( int col = colmin; col <=colmax; col++)

					{
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if(PixelValue==255)
							leftcount++;
						if(row==rowmin||col==colmin||row==rowmin+30||col==colmax)
							thumbbox.at<unsigned char>(row, col)=125;
					}
			}

		for ( int row = rowmax-30; row <=rowmax; row++)
			{
				for ( int col = colmin; col <=colmax; col++)

					{
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if(PixelValue==255)
							rightcount++;
						if(row==rowmax-30||col==colmin||row==rowmax||col==colmax)
							thumbbox.at<unsigned char>(row, col)=125;
					}
			}
		}
		if(flag==1)
		{
		for ( int row = rowmin; row <=rowmax; row++)
			{
				for ( int col = colmin; col <=colmin+30; col++)

					{
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if(PixelValue==255)
							leftcount++;
						if(row==rowmin||col==colmin||row==rowmax||col==colmin+30)
							thumbbox.at<unsigned char>(row, col)=125;
					}
			}

		for ( int row = rowmin; row <=rowmax; row++)
			{
				for ( int col = colmax-30; col <=colmax; col++)

					{
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if(PixelValue==255)
							rightcount++;
						if(row==rowmin||col==colmax-30||row==rowmax||col==colmax)
							thumbbox.at<unsigned char>(row, col)=125;
					}
			}
		}
		imshow("THUMB BOX",thumbbox);
		float ratioleft=(float)leftcount/(float)pixelcount;
		float ratioright=(float)rightcount/(float)pixelcount;
		if(flag==1)
		{
		printf("leftcount=%d\n",leftcount);
		printf("ratioleft=%f\n",ratioleft);
		printf("rightcount=%d\n",rightcount);
		printf("ratioright=%f\n",ratioright);
		}
		if(flag==0)
		{
		printf("topcount=%d\n",leftcount);
		printf("ratiotop=%f\n",ratioleft);
		printf("bottomcount=%d\n",rightcount);
		printf("ratiobottom=%f\n",ratioright);
		}

		if(ratioleft<0.05)
		{
			if(flag==1)
			printf("THUMB PRESENT(left box)\n");
			else
				printf("THUMB PRESENT(top box)\n");
		}
		if(ratioright<0.05)
		{
			if(flag==1)
			printf("THUMB PRESENT(right box)\n");
			else
				printf("THUMB PRESENT(bottom box)\n");
		}
		if((ratioleft>0.05&&ratioright>0.05)||(ratioleft<0.05&&ratioright<0.05))
	   {	printf("THUMB IS NOT PRESENT\n");
		    thumb = 0;
          }

		return thumb;
 
}

int edge_detection(Mat pad,Mat edgeoutput)
{
	int edge=0;
	for (int row = 1; row < pad.rows-1; row++)
	{
		for (int col = 1; col < pad.cols-1; col++)
		{
						unsigned char PixelValue = pad.at<unsigned char>(row, col);
						unsigned char PixelTop = pad.at<unsigned char>(row-1, col);
						unsigned char PixelLeft = pad.at<unsigned char>(row, col-1);
						unsigned char PixelRight = pad.at<unsigned char>(row, col+1);
						unsigned char PixelBottom = pad.at<unsigned char>(row+1, col);

						if (PixelValue==255)
						{
							if(PixelLeft==0 || PixelTop==0 || PixelRight==0 || PixelBottom==0)
							{	
								edgeoutput.at<unsigned char>(row, col)=125;
								edge++;
							}

						}
		}
	}
	for (int row = 1; row < pad.rows-1; row++)
	{
		for (int col = 1; col < pad.cols-1; col++)
		{
						unsigned char PixelValue = edgeoutput.at<unsigned char>(row, col);
						if (PixelValue==255)
							edgeoutput.at<unsigned char>(row, col)=0;

		}
	}
return (edge);
}

void centroid(Mat smoothimage,unsigned int *com_x,unsigned int *com_y)
{
unsigned int sum1=0,sum2=0,sum3=0;
	for ( int row = 0; row <smoothimage.rows; row++)
	{
				for ( int col = 0; col <smoothimage.cols; col++)

					{ 
						unsigned char PixelValue = smoothimage.at<unsigned char>(row, col);
						if (PixelValue==255)
						{
							sum1=sum1+(row*PixelValue);
							sum2=sum2+(col*PixelValue);
							sum3=sum3+PixelValue;
						}
				}
	}

	*com_x=sum1/sum3;
	*com_y=sum2/sum3;
	printf("CENTROID_X=%d   CENTROID_Y=%d\n",*com_x,*com_y);
}

void on_trackbar ( int ,void *)
{

	char trackbar[50];
	char * filename;

	printf("/***************************************************************************************/ \n");
	switch(position)
	 {
		case 0: filename	=	"A15.jpg";
		 break;
		case 1: filename	=	"A16.jpg";
		 break;
		case 2: filename	=	"A17.jpg";
		 break;
		case 3: filename	=	"A18.jpg";
		 break;
		case 4: filename	=	"A19.jpg";
		 break;
		case 5: filename	=	"A20.jpg";
		 break;
		case 6: filename	=	"A21.jpg";
		 break;
		case 7: filename	=	"A22.jpg";
		 break;
	    case 8: filename	=	"A23.jpg";
		 break;
		case 9: filename	=	"A24.jpg";
		 break;
	   case 10: filename	=	"A25.jpg";
		 break;
		case 11: filename	=	"A26.jpg";
		 break;
		case 12: filename	=	"A27.jpg";
		 break;
		case 13: filename	=	"A28.jpg";
		 break;
	    case 14: filename	=	"A36.jpg";
		 break;
		case 15: filename	=	"A39.jpg";
		 break;
		case 16: filename	=	"A40.jpg";
		 break;
		case 17: filename	=	"A41.jpg";
		 break;
		case 18: filename	=	"A42.jpg";
		 break;
		case 19: filename	=	"A43.jpg";
		 break;
		case 20: filename	=	"A44.jpg";
		 break;
		case 21: filename	=	"A45.jpg";
		 break;
		case 22: filename	=	"A46.jpg";
		 break;
		case 23: filename	=	"A13.jpg";
		 break;
		case 24: filename	=	"A14.jpg";
		 break;
	}

	InputFrame = imread (filename, CV_LOAD_IMAGE_UNCHANGED);
	imshow ("Input", InputFrame);

	/********************RGB TO YCRCB CONVERSION**********************/	
	if(InputFrame.channels()==3)
	{
	cvtColor(InputFrame,YF, CV_BGR2YCrCb); 
	split(YF,ycrcb);
	}
	imshow("converted",YF);
	
	/**************SEGMENTATION*********************/
	segmentedframe = ycrcb[1].clone ();
	Mat temp=ycrcb[1].clone ();

   // int threshold;
	//threshold=otsu_threshold(ycrcb[1]);
	//printf("\nthreshold = %d\n",threshold);
	threshold(ycrcb[1],segmentedframe, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	int pixelcount=0;
	for ( int row = 0; row <segmentedframe.rows; row++)
	{
				for ( int col = 0; col <segmentedframe.cols; col++)

					{ 
						unsigned char PixelValue = segmentedframe.at<unsigned char>(row, col);
						if (PixelValue==255)
							pixelcount++;
				}
	}
	//pixelcount=segmentation(temp,segmentedframe,threshold);
	printf("pixelcount=%d\n",pixelcount);
	imshow ("Output", segmentedframe);
	imwrite("segmented frame.jpg",segmentedframe);

	/**********************SMOOTHING OPERATION********************************************/
	Mat element = getStructuringElement( MORPH_ELLIPSE,Size(15,15),Point(7,7) );
	erode(segmentedframe,smoothimage, element );
	dilate( smoothimage,smoothimage, element );
	medianBlur ( smoothimage,smoothimage, 15 );
	imshow("Erosion_output",smoothimage);

  /************************ORIENTATION DETECTION*************************************/
	int rowmin,rowmax,colmin,colmax;
	rowmin=rowminimum(smoothimage);
	rowmax=rowmaximum(smoothimage);
	colmin=colminimum(smoothimage);
	colmax=colmaxmum(smoothimage);
	
	printf("rowmin=%d rowmax=%d colmin=%d colmax=%d \n",rowmin,rowmax,colmin,colmax);
	int flag =orientation_detection(rowmin,rowmax,colmin,colmax);
	
/**********************BOUNDARY MATRIX*************************/
	boundaryoutput = smoothimage.clone ();
	boundary_matrix(boundaryoutput,rowmin,rowmax,colmin,colmax);
	imshow("BOUNDARY_output",boundaryoutput);
		
/**************THUMB DETECTION*******************/
	Mat thumbbox=smoothimage.clone();
	int thumb = thumb_detection(smoothimage,thumbbox,flag,pixelcount,rowmin,rowmax,colmin,colmax);
		
/********************EGDE DETECTION***************************************************/
		Mat pad,edgeoutput;
		copyMakeBorder(smoothimage,pad,1,1,1,1,BORDER_CONSTANT,Scalar(0));
		imshow("PAD",pad);
		edgeoutput = pad.clone ();
		int edge=edge_detection(pad,edgeoutput);
		printf("edgepixel=%d\n",edge);

/************************CENTROID************************/
	unsigned int com_x=0,com_y=0;
	centroid(smoothimage,&com_x,&com_y);
	edgeoutput.at<unsigned char>(com_x, com_y)=125;
	//imshow("EDGE_output",edgeoutput);

	/**********************finger detection********************************/
	fingerdetection(flag,colmin,colmax,rowmin,rowmax,com_x,com_y,edgeoutput,thumb);





}

 
void classification (int orient ,int fingers,int tipcol[10],int tiprow[10], int distance,int thumb)
{

	int lf=0,rf=0,mf=0,ff=0; // here we are assuming that all 4 fingers are closed
	int dif,dif2;

	if (orient ==1)
	{
	if (fingers ==4)
	{ lf=1;
	  rf=1;
	  mf=1;
	  ff=1;
	}  /** all 4 fingers raised case **/

	/***************************************************************************************************/
	if (fingers ==2)
	{
	 
      /** case can be 0011 & 1001 **/ 	
	   dif = tipcol[1] - tipcol[0];
	
	   if (dif >75)
	   { 
		   lf=1;
		   ff=1;
	   }

	   if (dif <=75 )
	   {
		   mf=1;
		   ff=1;
	   }
	}// end 

	/***************************************************************************************************/


	if (fingers == 3)
	{ 
		/** our case possibilities are 0111 and 1110 **/
		dif = tipcol[1] - tipcol[0];
	    dif2 = tipcol[2] - tipcol[1];
	
      if( (dif < 50 )&&(dif2 > 55))
	  {
		  rf=1;
		  mf=1;
		  ff=1;
	  }

	  if( (dif > 55 )&&(dif2 < 50))
	  {   lf=1;
		  rf=1;
		  mf=1;
	  } // end
	}

	/***************************************************************************************************/


	if (fingers == 1)
	{
		// in our case it is either the left finger and first finger

		if (distance > 150)
		{
			ff=1;
		}
		else
		{
		   lf =1;
		}
}// end for finger 1
	
	
}
else	
{   
	 if (fingers ==4)
	{ lf=1;
	  rf=1;
	  mf=1;
	  ff=1;
	}  /** all 4 fingers raised case **/

	/***************************************************************************************************/


	if (fingers ==2)
	{
	 
      /** case can be 0011 & 1001 **/ 	
	   dif = tiprow[1] - tiprow[0];
	
	   if (dif >75)
	   { 
		   lf=1;
		   ff=1;
	   }

	   if (dif <= 75 )
	   {
		   mf=1;
		   ff=1;
	   }
	}// end 

	/***************************************************************************************************/


	if (fingers == 3)
	{ 
		/** our case possibilities are 0111 and 1110 **/
		dif = tiprow[1] - tiprow[0];
	    dif2 = tiprow[2] - tiprow[1];
	
      if( (dif < 50 )&&(dif2 > 55))
	  {
		  rf=1;
		  mf=1;
		  ff=1;
	  }

	  if( (dif > 55 )&&(dif2 < 50))
	  {   lf=1;
		  rf=1;
		  mf=1;
	  } // end
	}

	/***************************************************************************************************/

	if (fingers == 1)
	{
		// in our case it is either the left finger and first finger

		if (distance > 150)
		{
			ff=1;
		}
		else
		{
		   lf =1;
		}
}// end for finger 1
		
	/***************************************************************************************************/
}

printf("\n printing the finger pattern :-  %d %d %d %d %d \n ",lf,rf,mf,ff,thumb);

}


/*********************** END OF THE PROGRAM ****************************************/
/************************ THANK YOU**********************************************/