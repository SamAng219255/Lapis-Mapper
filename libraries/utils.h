#define TRUE 1
#define FALSE 0

double clamp(double x,double min,double max) {
	return max<x ? max : (min>x?min:x);
}

int intFromStr(char* str) {
	int y=0;
	int i=0;
	int isNegative=0;
	char cur=str[i];
	while(cur!='\x00') {
		if(i==0 && cur=='-') {
			isNegative=1;
		}
		else {
			if(y>214748364 || (y==214748364 && (cur=='8' || cur=='9'))) {
				return 2147483647;
				/*
				If:
					y is large enough that the decimal shift will overflow it OR
					we're adding an '8' or '9' where the max without overflow is a '7'
				Then:
					return the max int
				*/
			}
			y*=10;
			switch(cur) {
				case '0':
					y+=0;
					break;
				case '1':
					y+=1;
					break;
				case '2':
					y+=2;
					break;
				case '3':
					y+=3;
					break;
				case '4':
					y+=4;
					break;
				case '5':
					y+=5;
					break;
				case '6':
					y+=6;
					break;
				case '7':
					y+=7;
					break;
				case '8':
					y+=8;
					break;
				case '9':
					y+=9;
					break;
				default:
					return 0;
					break;
			}
		}
		i++;
		cur=str[i];
	}
	if(isNegative) y*=-1;
	return y;
}

int numPlaces(int n) {
	if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	return 10;
}

size_t fwriteE(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	void* nptr=malloc(nmemb*size);
	for(size_t i=0; i<nmemb; i++) {
		size_t ind=i*size;
		for(size_t j=0; j<size; j++) {
			((uint8_t*)nptr)[ind+size-j-1]=((uint8_t*)ptr)[ind+j];
		}
	}
	size_t result=fwrite(nptr,size,nmemb,stream);
	free(nptr);
	return result;
}
size_t freadE(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	void* nptr=malloc(nmemb*size);
	size_t result=fread(nptr,size,nmemb,stream);
	for(size_t i=0; i<result; i++) {
		size_t ind=i*size;
		for(size_t j=0; j<size; j++) {
			//printf("(%zu/%zu, %zu/%zu): %02x\n",i,result,j,size,((uint8_t*)nptr)[ind+j]);
			((uint8_t*)ptr)[ind+j]=((uint8_t*)nptr)[ind+size-j-1];
		}
	}
	free(nptr);
	return result;
}