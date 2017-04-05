#include <stdio.h>
#include <string.h>
#include <intrin.h>
#include <process.h>

#define BLOCK_WIDTH 8
#define BLOCK_HEIGHT 16
#define CHAR_COUNT 95

#pragma pack(push,1)

typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long   bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned long   bfOffBits;
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	unsigned long   biSize;
	long    biWidth;
	long    biHeight;
	unsigned short    biPlanes;
	unsigned short    biBitCount;
	unsigned long   biCompression;
	unsigned long   biSizeImage;
	long    biXPelsPerMerer;
	long    biYPelsPerMerer;
	unsigned long   biClrUsed;
	unsigned long   biClrImportant;
}BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
	unsigned char    rgbBlue;
	unsigned char    rgbGreen;
	unsigned char    rgbRed;
	unsigned char    rgbReserved;
}RGBQUAD;


typedef struct _color
{
	unsigned char    rgbBlue;
	unsigned char    rgbGreen;
	unsigned char    rgbRed;
}PIXEL;

typedef struct _block
{
	PIXEL pixel[16][8];
}BLOCK,*PBLOCK;

#pragma pack(pop)

unsigned char *pBmpBuf = NULL;//读入图像数据的指针  
int bmpWidth = 0;//图像的宽  
int bmpHeight = 0;//图像的高  
RGBQUAD *pColorTable = NULL;//颜色表指针  
int biBitCount = 0;//图像类型，每像素位数  

PBLOCK pBlock_abc = {};

char abc[] = { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
'0','1','2','3','4','5','6','7','8','9',
'~','`','!','@','#','$','%','^','&','*','(',')','_','+','-','=','{','}','|','[',']','\\',':','\"',';','\'','<','>','?',',','.','/',' ' };

char abc_s[] = { 'x', 'O', 'X', 'v', '0', '8',
'~', '`', '#', '*', '.', '\\', '/', ':',  '^', '_', '+', '|', ' '
};

unsigned int ch_count;
int abc_i[CHAR_COUNT+1] = { -1 };
int abc_gray[CHAR_COUNT+1];
int abc_vec[CHAR_COUNT+1];

int getMinPos(int* valueList, int count)
{
	if (valueList == NULL || count == 0)
		return 0;

	int tmp = valueList[0];	
	int pos = 0;
	for (int i=0; i<count-1; i++)
	{
		for (int j=i+1; j<count; j++)
		{
			if (valueList[j]<tmp)
			{
				tmp = valueList[j];
				pos = j;
			}
		}
	}
	return pos;
}

int getMinValue(int* valueList, int count)
{
	if (valueList == NULL || count == 0)
		return 0;

	int tmp = valueList[0];	
	for (int i=0; i<count-1; i++)
	{
		for (int j=i+1; j<count; j++)
		{
			if (valueList[j]<tmp)
			{
				tmp = valueList[j];
			}
		}
	}
	return tmp;
}

int* getVecList(PBLOCK pBlockList, int count)
{
	if(pBlockList==NULL || count==0)
		return NULL;

	int up=0,down=0,left=0,right=0;
	//int mh = BLOCK_WIDTH*105*3*255/2;
	//int mw = BLOCK_HEIGHT*28*3*255/2;
	int* vl = new int[count];
	memset((char*)vl, 0, count*sizeof(int));
	for (int c=0; c<count; c++)
	{
		up=0;down=0;left=0;right=0;
		for (int h=1; h<BLOCK_HEIGHT-1; h++)
		{
			for (int w=0; w<BLOCK_WIDTH; w++)
			{
				int gray_pic = pBlockList[c].pixel[h][w].rgbBlue + 
					pBlockList[c].pixel[h][w].rgbGreen + 
					pBlockList[c].pixel[h][w].rgbRed;
				if (h > 7)
				{
					if (w > 3)
						down += gray_pic*(h - 7.5)*(w - 3.5);
					else
						up += gray_pic*(h - 7.5)*(3.5 - w);
				}
				else
				{
					if (w > 3)
						left += gray_pic*(7.5 - h)*(w - 3.5);
					else
						right += gray_pic*(7.5 - h)*(3.5 - w);
				}

				//if (h>7) down+=gray_pic*(h-7.5);
				//else up+=gray_pic*(h-7.5);
				//if (w>3) right+=gray_pic*(w-3.5);
				//else left = left + gray_pic*(w-3.5);
			}
		}
		//up = up*(255.0/mh);
		//down = down*(255.0/mh);
		//left = left*(255.0/mw);
		//right = right*(255.0/mw);
		int max = 149926;
		up = up*(255.0 / max);
		down = down*(255.0 / max);
		left = left*(255.0 / max);
		right = right*(255.0 / max);
		vl[c] = (up&0xff) + ((down&0xff)<<8) + ((left&0xff)<<16) + ((right&0xff)<<24);
	}

	return vl;
}
int getVecMatchValue(int vec1,int vec2)
{
	char* cvec1 = (char*)&vec1;
	char* cvec2 = (char*)&vec2;
	int tmp1 = pow(abs(cvec1[0]-cvec2[0]),2);
	int tmp2 = pow(abs(cvec1[1]-cvec2[1]),2);
	int tmp3 = pow(abs(cvec1[2]-cvec2[2]),2);
	int tmp4 = pow(abs(cvec1[3]-cvec2[3]),2);
	int v = abs(tmp1 - tmp2) + abs(tmp1 - tmp3) + abs(tmp1 - tmp4) +
		abs(tmp2 - tmp3) + abs(tmp2 - tmp4) + abs(tmp3 - tmp4);
	return v;
}

int* getGrayList(PBLOCK pBlockList, int count, int start_gray, int end_gray)
{
	if(pBlockList==NULL || count==0 || 
		start_gray<0 || start_gray>255 ||
		end_gray<0 || end_gray>255 ||
		end_gray < start_gray)
		return NULL;

	int* gl = new int[count];
	memset((char*)gl, 0, count*sizeof(int));	
	int gray = 0;
	for (int c=0; c<count; c++)
	{
		for (int h=1; h<BLOCK_HEIGHT-1; h++)
		{
			for (int w=0; w<BLOCK_WIDTH; w++)
			{
				gray = (pBlockList[c].pixel[h][w].rgbBlue + 
					pBlockList[c].pixel[h][w].rgbGreen + 
					pBlockList[c].pixel[h][w].rgbRed)/3;

				gray = gray*((end_gray-start_gray)/255.0)+start_gray;// 控制灰度值在（start_gray~end_gray）之间
				gl[c] += gray;
			}
		}
		gl[c] /= (BLOCK_HEIGHT - 2)*BLOCK_WIDTH;
	}
	return gl;
}
int* matchGray(int* grayList, int lcount, int gray, int mcount)
{
	if (grayList == NULL || lcount<mcount)
		return NULL;

	int* pos = new int[mcount];
	int* gl = new int[lcount];
	memcpy(gl,grayList,lcount*sizeof(int));

	for (int lc=0; lc<lcount; lc++)
	{
		gl[lc] = abs(gl[lc]-gray);
	}

	int tmp;	
	for (int i=0; i<mcount; i++)
	{
		for (int j=i+1; j<lcount; j++)
		{
			if (gl[j]<gl[i])
			{
				tmp = gl[i];
				gl[i] = gl[j];
				gl[j] = tmp;
			}
		}
	}

	for (int i=0; i<mcount; i++)
	{
		for (int j=0; j<lcount; j++)
		{
			for (int k=0; k<i; k++)
			{
				if(pos[k]==j)
					goto match_gray_1;
			}
			if (gl[i] == abs(grayList[j] - gray))
			{
				pos[i] = j;
			}
match_gray_1:
			continue;
		}
	}
	return pos;
}
int getGrayMatchValue(int gray1,int gray2)
{
	return abs(gray1-gray2);;
}

char matchChar(PBLOCK pBlock)
{
	//pBlock must have 8*16 pixels(24bit per pixel)
	if (pBlock == NULL)
		return ' ';

	// match gray
	int mcount = 2;
	int* gray = getGrayList(pBlock,1,147,255);
	if (gray[0] > 240)
		gray[0] = 255;
	int* gray_pos_list = matchGray(abc_gray, ch_count,gray[0],mcount);

	// match vector 
	int* vec = getVecList(pBlock,1);
	int* mv = new int[mcount];
	for (int i=0; i<mcount; i++)
	{
		mv[i] = getVecMatchValue(abc_vec[gray_pos_list[i]],vec[0]);
	}

	// get the min
	int pos = getMinPos(mv,mcount);

	return abc_s[gray_pos_list[pos]];
}

bool readBmp(char *bmpName, PBLOCK &pBlock, int &w_count, int &h_count)
{
	FILE *fp=fopen(bmpName,"rb");
	if(fp==0) return false;  

	fseek(fp, sizeof(BITMAPFILEHEADER),0);  	
	BITMAPINFOHEADER head;    
	fread(&head, sizeof(BITMAPINFOHEADER), 1,fp); //获取图像宽、高、每像素所占位数等信息  
	if(head.biBitCount!=24)  
		return false;

	int lineByte=(head.biWidth * 3+3)/4*4;
	int gap = lineByte - head.biWidth * 3;
	unsigned char* pBmpBuf = new unsigned char[lineByte * head.biHeight];  
	if (pBmpBuf == NULL) return false;
	fread(pBmpBuf,1,lineByte * head.biHeight,fp);  
	fclose(fp);//关闭文件  

	w_count = head.biWidth/BLOCK_WIDTH + (head.biWidth % BLOCK_WIDTH ? 1:0);
	h_count = head.biHeight/BLOCK_HEIGHT + (head.biHeight % BLOCK_HEIGHT ? 1:0);
	pBlock = new BLOCK[w_count * h_count]; 
	if (pBlock == NULL)return false;
	memset(pBlock,0xff,w_count * h_count * sizeof(BLOCK));
	int w,h;
	int rw,rh;
	for (h=0; h<h_count; h++)
	{
		rh = ((head.biHeight-h*BLOCK_HEIGHT) < BLOCK_HEIGHT) ? head.biHeight-h*BLOCK_HEIGHT:BLOCK_HEIGHT;
		for (w=0; w<w_count; w++)
		{
			rw = ((head.biWidth-w*BLOCK_WIDTH) < BLOCK_WIDTH) ? head.biWidth-w*BLOCK_WIDTH:BLOCK_WIDTH;
			for (int i=0; i<rh; i++)
			{
				memcpy((char*)(pBlock+h*w_count+w)+i*BLOCK_WIDTH*sizeof(PIXEL), 
					pBmpBuf+h*BLOCK_HEIGHT*lineByte+i*lineByte+w*BLOCK_WIDTH*sizeof(PIXEL), 
					rw*sizeof(PIXEL));
			}
		}
	}

	delete pBmpBuf;
	return true;
}

void usage()
{
	printf("Usage: pic2char xxx.bmp\n");
	exit(0);
}

int main(int argc, char** argv)
{
	if (argc !=2)
		usage();
	if (memcmp(argv[1]+strlen(argv[1])-4,".bmp",4)!=0)
		usage();

	// 
	ch_count = sizeof(abc_s);
	for (size_t j = 0; j < ch_count; j++)
	{
		for (size_t i = 0; i < CHAR_COUNT; i++)
		{
			if (abc[i] == abc_s[j])
			{
				abc_i[j] = i;
				break;
			}
		}
		if (abc_i[j] == -1)
		{
			printf("init error\n");
			return 0;
		}
	}		

	// 读取bmp文件
	PBLOCK pBlock_pic = NULL;
	int wCount_abc = 0, hCount_abc = 0;
	int wCount_pic = 0, hCount_pic = 0;
	if (!readBmp("abc.bmp",pBlock_abc,wCount_abc,hCount_abc))
	{
		printf("read error: abc.bmp\n");
		return 0;
	}
	if (!readBmp(argv[1],pBlock_pic,wCount_pic,hCount_pic))
	{
		printf("read error: %s\n",argv[1]);
		return 0;
	}

	// 获取abc的灰度和方向
	int* gray = getGrayList(pBlock_abc,CHAR_COUNT,0,255);
	int* vec = getVecList(pBlock_abc,CHAR_COUNT);
	for (size_t i = 0; i < ch_count; i++)
	{
		abc_gray[i] = gray[abc_i[i]];
		abc_vec[i] = vec[abc_i[i]];
	}

	// pic 灰度处理 字母匹配
	unsigned char* chars = new unsigned char[wCount_pic*hCount_pic];
	for(int h=0; h<hCount_pic; h++)
	{
		for(int w=0; w<wCount_pic; w++)
		{
			chars[h*wCount_pic + w] = matchChar(pBlock_pic + h*wCount_pic + w);
		}
	}	

	// txt文件输出
	char* dst_name = new char[strlen(argv[1])+1];
	memset(dst_name,0,strlen(argv[1])+1);
	memcpy(dst_name,argv[1],strlen(argv[1]));
	memcpy(dst_name+strlen(argv[1])-3,"txt",3);
	FILE *fp=fopen(dst_name,"w");	
	if (fp==NULL) return 0;
	for(int h=0; h<hCount_pic; h++)
	{
		for(int w=0; w<wCount_pic; w++)
		{
			fwrite(chars+(hCount_pic-h-1)*wCount_pic + w,1,1,fp);
		}
		fwrite("\n",1,1,fp);
	}
	fclose(fp);

	delete chars;
	delete[] pBlock_abc;
	delete[] pBlock_pic;
	printf("Generate successful: %s\n",dst_name);
	delete dst_name;
	//getchar();
}