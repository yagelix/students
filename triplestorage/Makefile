STORAGE_IMPL=list_impl


all: test1 test2 test3

test1: test1.o tstorage_$(STORAGE_IMPL).o
	cc -o $@ $^
	
test2:	test2.o tstorage_$(STORAGE_IMPL).o
	cc -o $@ $^

test3:	test3.o tstorage_$(STORAGE_IMPL).o
	cc -o $@ $^
