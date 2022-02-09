#! /bin/bash

fmyt=$(find mytar)
ftmp=$(find tmp)
salida=1

if [[ -z "$fmyt" ]]; then
	echo "Ha habido un error, no se ha encontrado ./mytar."
else 
	if [[ -z "$ftmp" ]]; then
		echo "No hay carpeta temporal"
	else
		rm -r tmp
	fi

	mkdir tmp
	cd tmp
	hdi=$(head /etc/passwd)
	hdl=$(head -c 1024 /dev/urandom)

	echo "Hello world!" > file1.txt
	echo $hdi > file2.txt
	echo $hdl  file3.txt

	../mytar -c -f filetar.mtar file1.txt file2.txt file3.txt

	mkdir out
	cp filetar.mtar ./out
	cd out
	../../mytar -x -f filetar.mtar

	if (diff ./file1.txt ../file1.txt) && (diff ./file2.txt ../file2.txt) && (diff ./file3.txt ../file3.txt); then
		echo "Correct."
		salida=0
	else
		cd ../..
		echo "Algo fue mal en la extracción."
	fi
fi
exit $salida