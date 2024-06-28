#include <SDL2/SDL.h>
#include <squirrel.h>

HSQUIRRELVM v;

SQInteger file_lexfeedASCII(SQUserPointer file)
{
    int ret;
    char c;
    if( ( ret=fread(&c,sizeof(c),1,(FILE *)file )>0) )
        return c;
    return 0;
}

int compile_file(HSQUIRRELVM v,const char *filename)
{
    FILE *f=fopen(filename,"rb");
    if(f)
    {
         sq_compile(v,file_lexfeedASCII,f,filename,1);
         fclose(f);
         return 1;
    }
    return 0;
}

int main()
{
    v = sq_open(1024);

    




    sq_close(v);

    return 0;
}
