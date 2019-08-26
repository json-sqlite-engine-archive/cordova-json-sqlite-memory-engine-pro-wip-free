all: sqlite3 ndkbuild

ndkbuild: sqlite3
	make -C ndk ndkbuild

ndkregen:
	make -C ndk regen

test: sqlite3
	rm -f ./ctest
	cc ctest.c json-sqlite-engine-wrapper-pro.c sqlite3/sqlite3.c -o ctest
	./ctest

sqlite3:
	mkdir -p sqlite3
	rm -rf sqlite-amalgamation-*
	curl -O https://www.sqlite.org/2019/sqlite-amalgamation-3280000.zip
	unzip sqlite-amalgamation-3280000.zip
	mv sqlite-amalgamation-3280000/sqlite3.* sqlite3

clean:
	make -C ndk clean
	rm -rf sqlite-amalgamation-*
	rm -rf sqlite3
	rm -rf ctest

