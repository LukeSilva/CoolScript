cd source
gcc -c *.c
cd lib
gcc -c *.c
mv *.o ../
cd ..
gcc *.o
mv a.out ../
rm *.o
cd ..
