for x in `seq 1 10`; do for k in {2,16,128,1024}; do for i in lfq; do for j in {0,1,2,3}; do python changing_memory.py $j $i $k; done; done; done; done
