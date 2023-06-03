#include <stdio.h>
#include <string.h>
#include <fstream>
#include "md5.h"

int main(int argc, char *argv[])
{
    switch (argc)
    {
    case 1:
        printf("Usage: %s <command>\n", argv[0]);
        MyMD5::print_help();
        break;
    case 2:
        if (strcmp(argv[1], "-t") == 0)
        {
            printf("Starting test\n");
            MyMD5 m;
            printf("'' md5: %s\n", m.md5("").c_str());
            printf("'a' md5: %s\n", m.md5("a").c_str());
            printf("'abc' md5: %s\n", m.md5("abc").c_str());
            printf("'message digest' md5: %s\n", m.md5("message digest").c_str());
            printf("'abcdefghijklmnopqrstuvwxyz' md5: %s\n", m.md5("abcdefghijklmnopqrstuvwxyz").c_str());
            printf("'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789' md5: %s\n",
                   m.md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789").c_str());
            printf("'12345678901234567890123456789012345678901234567890123456789012345678901234567890' md5: %s\n",
             m.md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890").c_str());
        }
        else if (strcmp(argv[1], "-v") == 0)
        {
            MyMD5 m;
            printf("Version: %s\n", m.get_version().c_str());
        }
        else if (strcmp(argv[1], "-h") == 0)
        {
            printf("Usage: %s <command>\n", argv[0]);
            MyMD5::print_help();
        }
        else
        {
            printf("\033[31mUnknown Command \033[0m\n");
            MyMD5::print_help();
        }
        break;
    case 3:
        if (strcmp(argv[1], "-f") == 0)
        {
            // md5 a file
            MyMD5 m;
            printf("%s  %s\n", m.md5file(argv[2]).c_str(), argv[2]);
        }
        else if (strcmp(argv[1], "-m") == 0)
        {
            // md5 a message
            MyMD5 m;
            printf("%s\n", m.md5(argv[2], strlen(argv[2])).c_str());
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            // check a file ended with .md5
            std::ifstream file(argv[2]);
            if (file.is_open())
            {
                MyMD5 m;
                std::string line;
                std::getline(file, line); // read the first line
                file.close();
                size_t pos = line.find("  ");
                if (pos != std::string::npos)
                {
                    std::string md5_in_file = line.substr(0, pos); // the hash to be checked
                    std::string filename = line.substr(pos + 2);
                    std::string true_md5 = m.md5file(filename.c_str());
                    printf("Md5 in the file '%s' : %s\n", argv[2], md5_in_file.c_str());
                    printf("Actully md5 value of '%s' : %s\n", filename.c_str(), true_md5.c_str());
                    if (md5_in_file == true_md5)
                    {
                        printf("\033[32mFile %s is passed!\033[0m\n", filename.c_str());
                    }
                    else
                    {
                        printf("\033[31mFile %s is not passed!\033[0m\n", filename.c_str());
                    }
                }
            }
            else
            {
                printf("\033[31mCan't open file %s\033[0m\n", argv[2]);
            }
        }
        else
        {
            printf("\033[31mUnknown Command \033[0m\n");
            MyMD5::print_help();
        }
        break;
    default:
        printf("\033[31mUnknown Command \033[0m\n");
        MyMD5::print_help();
    }
}