export PATH=/home/qmao/Android/resources/Tools/ARM_CrossCompiler/arm-2011.03/bin:${PATH}
arm-none-linux-gnueabi-gcc -I. -I./include -static -Wall -o synaptics_production_test synaptics_production_test.c libxls.a
