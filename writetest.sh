#!/bin/bash -x

# write test
function usage {
    cat <<EOF
Usage:
$0 output_filename min_page_size step_size max_page_size
$0 output_filename min_page_size max_page_size
$0 output_filename max_page_size
$0 output_filename
$0
  - Page size unit is 100 bytes.
EOF
}

# string 100 bytes long
str="123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 "

# make the test
make writetest

case $1 in 
    -h|--help|-*|[0-9]*) 
	usage
	exit
	;;
    *)
	filename=${1:-testout}
	echo "The basename of the output file is ${filename}."
	shift;
	;;
esac

number_re='^[0-9]+$'
for arg in $@; do
    echo "testing ${arg}"
    if ! [[ $arg =~ ${number_re} ]]; then usage; exit 1; fi
done

# if the rest of the arguments are not accepted by seq,
# then something is wrong.
if ! seq ${@:-1} &> /dev/null; then usage; exit 1; fi

case $# in 
    0)
	maxPageSize=10
	stepSize=1
	minPageSize=1
	;;
    1)
	maxPageSize=$1
	stepSize=1
	minPageSize=1
	;;
    2)
	maxPageSize=$2
	stepSize=1
	minPageSize=$1
	;;

    3)
	maxPageSize=$3
	stepSize=$2
	minPageSize=$1
	;;
    *)  
	usage
	exit 1 
	;;
esac

echo "Running simulation with minPageSize ${minPageSize}, stepSize ${stepSize}, maxPageSize ${maxPageSize}"

echo "Putting tabular results in ${filename}.pages (varying number of pages of fixed size) and ${filename}.page_sizes (one page of varying sizes)."

echo -e "page_size\tmmap_paging\twrite_paging" > ${filename}.pages
for pages in $(seq ${minPageSize} ${stepSize} ${maxPageSize}); do
    output=${filename}.${pages}.1
    ./writetest ${output} ${pages} 1 ${str} | tee -a ${filename}.pages 
    # check to make sure that the writetest output files are the same, then remove them
    diff ${output}.mmap ${output}.write
    rm ${output}.mmap ${output}.write
done
echo -e "page_size\tmmap\twrite" > ${filename}.page_sizes
for page_size in $(seq ${minPageSize} ${stepSize} ${maxPageSize}); do
    output=${filename}.1.${page_size}
    ./writetest ${output} 1 ${page_size} ${str} | tee -a ${filename}.page_sizes
    diff ${output}.mmap ${output}.write
    rm ${output}.mmap ${output}.write
done
gnuplot -p <<EOF
# set terminal wxt
set terminal png
set output "${filename}.png"
set xlabel "Bytes"
set ylabel "Seconds"
plot for[col=2:3] "${filename}.pages" using 1:col title columnheader(col) with lines, \
     for[col=2:3] "${filename}.page_sizes" using 1:col title columnheader(col) with lines
EOF
eog "${filename}.png"

