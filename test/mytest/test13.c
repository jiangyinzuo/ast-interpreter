extern int GET();
extern void * MALLOC(int);
extern void FREE(void *);
extern void PRINT(int);

   
int  fibonacci(int b) {
   int i;
   int c;
   int a[2];

   if (b < 2)
      return b;
   for (i=0; i< 2; i=i+1)
   {
       a[i] = b-1-i;
   }
   c = fibonacci(a[0]) + fibonacci(a[1]);PRINT(c);
   return c;
}  
   
int main() {
   int a;
   int b;
   a = 5;

   b = fibonacci(a+1);
   PRINT(b);
   return 0;
}