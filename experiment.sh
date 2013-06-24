trace="traces/fakegcc.trace" 

echo $trace
printf "K0\tK1\tK2\tM\tD\tF\tIPC\n"

for k0 in 1 2 3
do
  for k1 in 1 2 3
  do
    for k2 in 1 2 3
    do
      for m in 2 4 8
      do
        for d in 1 2 4
        do
          for f in 2 4 8
          do
            printf "$k0\t$k1\t$k2\t$m\t$d\t$f\t"
            ./procsim -d$d -j$k0 -k$k1 -l$k2 -f$f -m$m -i $trace
          done
        done
      done
    done
  done
done
