#!/bin/bash
#
# Author:	Pedro Valero
#
# Description: Script used to generate graphs from .csv files.
# (See help for more details)
#
# Date: 2/3/15
TMP="tmp.yml"
BODY="graph-files/body_template"
BEGIN="graph-files/begin"
VARS="graph-files/vars_and_funcs"

if [ $# -eq 0 ] || [ $# -eq 1 ]
	then
	echo "Usage: ./generate_graphs.sh [output] [csv_files]"
	echo "Generates a folder with the name [output] which contains the [data_files] and an index.html with the graphs related to [csv_files]"
	echo "The data files should have been generated by mist or have the same format"
	echo "Empty files will be ignored"
	echo "This script needs mustache to be installed"
	exit
fi


OUTPUT=$1
FILES=${@:2}
i=1


echo "Creating dir $OUTPUT..."
mkdir $OUTPUT

OUT_FILE="$OUTPUT/index.html"


echo "Preparing output file..."
cat $BEGIN  > $OUT_FILE

# Preparing the multiselect menu to choose which graphs are visible
echo "<div class=\"multiselect\">" >> $OUT_FILE
for file in $FILES
do
	if [[ -s $file ]]
	then
		base=${file##*/}
		echo "<label><input type=\"checkbox\" name=\"option[]\" onclick = \"change('#graph$i',this)\" checked/>$base</label>" >> $OUT_FILE

		i=$[i + 1]
	fi
done
echo "</div><br>" >> $OUT_FILE

# Writing divs, one per graph, with the results of mist
i=1
echo "Parsing files..."
for file in $FILES
do
	if [[ -s $file ]]
	then
		DESCRIPTION=$(tail -1 $file)
		CONCLUSION=$(tail -2 $file | head -1)
		TIME=$(tail -3 $file | head -1)

		if [[ $CONCLUSION == *"safe"* ]] # Check if the conclusion has been written or the execution hs been stopped by timeout
		then
			base=${file##*/}
			sed '$d' $file | sed '$d' | sed '$d' > $OUTPUT/$base

			echo "<div class=\"graph\" id=\"graph$i\"><h1>$base</h1><h3>$CONCLUSION ($TIME)</h3><h3>$DESCRIPTION</h3></div>" >> $OUT_FILE
		else
	  		base=${file##*/}
			sed '$d' $file  > $OUTPUT/$base

			echo "<div class=\"graph\" id=\"graph$i\"><h1>$base</h1><h3>Timeout</h3><h3>$DESCRIPTION</h3></div>" >> $OUT_FILE
		fi

		i=$[i + 1]
	else
		echo "File $file empty. It will be ignored"
	fi
done

cat $VARS >> $OUT_FILE

echo "" >> $OUT_FILE

echo "Building temporal yml file..."
echo "{ \"translator\" :[" >> $TMP

# Preparing .yml file with the needed translations. This file will
# be used by mustache
i=1
for file in $FILES
do
	if [[ -s $file ]]
	then
		if [ $i -eq 1 ]
		then
			echo "{\"graph\": \"graph$i\", \"file\": \"${file##*/}\", \"svg\": \"svg$i\"}" >> $TMP
		else
			echo "," >> $TMP
			echo "{\"graph\": \"graph$i\", \"file\": \"${file##*/}\", \"svg\": \"svg$i\"}" >> $TMP
		fi
		echo "" >> $TMP
		i=$[i + 1]
	fi
done

echo "]}" >> $TMP

echo "Building final document..."

cat $TMP | mustache - $BODY >> $OUT_FILE

echo "Removing temporal files..."
rm $TMP

exit

