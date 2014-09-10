

writetest: mkpath.h mkpath.cpp writeTest.cpp
	g++ mkpath.cpp writeTest.cpp -o writetest

test: writetest 
	for pages in $(shell seq 1 5 10); do \
	  output=page.$${pages} ;\
	  for page_size in $(shell seq 100 100 10000); do \
	    echo "pages $${pages}, page size $${page_size}"; \
	    ./writetest testout.$${pages}.$${page_size} $${pages} $${page_size} "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 " >> $${output}; \
	    diff testout.$${pages}.$${page_size}.* ; \
	    rm testout.$${pages}.$${page_size}.* ; \
	done; done


clean: 
	rm *.write
	rm *.mmap
