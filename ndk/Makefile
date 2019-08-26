all: ndkbuild

regen: gluegentools
	java -cp gluegentools/antlr.jar:gluegentools/gluegen.jar com.jogamp.gluegen.GlueGen -I. -Ecom.jogamp.gluegen.JavaEmitter -CJSQLCOREPRONDKWrapper.cfg ../json-sqlite-engine-wrapper-pro.h
	# comment out imports not needed:
	sed -i.orig 's/^import/\/\/import/' java/io/sqlc/JSQLCOREPRONDKWrapper.java

ndkbuild:
	rm -rf lib libs *.jar
	ndk-build
	cp -r libs lib
	jar cf json-sqlite-engine-procore-wip-ndk.jar lib

gluegentools:
	curl -LO https://github.com/sqlg/gluegentools/archive/master.zip
	unzip master.zip
	mv gluegentools-master gluegentools

clean:
	rm -rf gluegentools gluegentools-* java/io/sqlc/JSQL*.java.orig
	rm -rf java/io/sqlc/JSQL*.java.orig
	rm -rf obj lib libs
	rm -rf *.jar
	rm -rf *.zip
