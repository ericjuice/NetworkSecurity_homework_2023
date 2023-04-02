#include<stdio.h>
// generate a DES class 
class DES {
public:
    DES();
    DES(const char *key);
    DES(const DES &des);
    DES &operator=(const DES &des);
    ~DES();
};