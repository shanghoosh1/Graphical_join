# Graphical_join
A new efficient and scalable physical join algorithm on relational data.   
The algorithm is considered as a worst-case optimal join algorithm.  

The code has been tested on Ubuntu 18.10

# To compile:
First cd into the folder "Graphical_join-main" then run “make -f Makefile CONF=Release”
# The Data:
You can generate the TPCDS data by following the instructions provided in http://www.tpc.org/  
For the JOB data and queries refer to https://github.com/gregrahn/join-order-benchmark.  
For lastFM data refer to https://grouplens.org/datasets/hetrec-2011/  
Note, all the data files should be in .csv format including the headers as attribute names, and the deliminator should be '|'. All the data types should be integer. Make sure you give the same index per distinct value in all the tables.  
# To run:
CD into “Graphical_join-main/dist/Release/GNU-Linux” then run “./graphicaljoin” with the appropriate arguments.  
The main arguments are as follows:  
--input /path/to/source/CSVs  
--output /path/for/result  
--query q_name  we have already implemented a few queries  JOB_A, JOB_B, JOB_C, JOB_D, tpcds1,tpcds2, lastFM_A1, lastFM_A2, lastFM_B  
--gen_mode [1-5] the generation mode. If 5 is chosen, the join result will be generated in the memory and also the join summary will be stored in disk and then the join summary will be loaded in memory again and the de-summarization will be carried out. The Wall time will be provided per step.   
--max_init default=200000000 specifies the maximum vector size in de-summarization. set with smaller values if you have a small RAM.   
# An example: 
./graphicaljoin --input /path2integerCSVs/ --output ./output/ --query JOB_A --gen_mode 5 --max_init 200000000  
