# Multithreading-Vending-Machine
This program acts like a vending machine that have supplier and consumer. This program using pthread.  

### compile with GCC
```
$ gcc os2.c -o os2 -lpthread
```

### run by command
```
$ ./os2
```

### Configuration Files
There are 2 types of file.
1. supplierN.txt (N = 1-5)
2. consumerN.txt (N = 1-8)

It's mean that we have 5 suppliers selling different goods and 8 consumers who gonna bye our goods.
In the file we have 3 attributes seperate by \n <br />
First line => Name of product<br />
Seconde line => Interval time<br />
Third line => Repeat (if thread failed to supplie or consume more than this number, it will x2 the interval time)<br />
```
$ ./shell filename
```

This assignment was submitted to 'Operating System' subject, CE KMITL
