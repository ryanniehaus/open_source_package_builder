#!/usr/bin/env bash

wget https://github.com/ryanniehaus/heroku-bash-buildpack/raw/master/bin/dataURLFromCloudinary.py
chmod u+rx dataURLFromCloudinary.py

sudo -H pip install --upgrade pip
sudo -H pip install --upgrade ndg-httpsclient
sudo -H pip install cloudinary

eval "$(ssh-agent -s)"

#add the ssh key to the agent
git config --global user.email "ryan.niehaus@gmail.com"
git config --global user.name "Ryan Niehaus"
git config --global credential.https://github.com.username ryanniehaus
git config --global push.default simple

echo downloading $(./dataURLFromCloudinary.py id_rsa)
wget --no-cache $(./dataURLFromCloudinary.py id_rsa) &> /dev/null
echo downloading $(./dataURLFromCloudinary.py id_rsa.pub)
wget --no-cache $(./dataURLFromCloudinary.py id_rsa.pub) &> /dev/null

chmod og-rwx,u+rw id_rsa*

echo "#!/usr/bin/env expect" > `pwd`"/ps.sh"
echo "spawn ssh-add id_rsa" >> `pwd`"/ps.sh"
echo "expect \"Enter passphrase for id_rsa:\"" >> `pwd`"/ps.sh"
echo "send \"$RSA_PASSPHRASE\n\";" >> `pwd`"/ps.sh"
echo "interact" >> `pwd`"/ps.sh"
echo >> `pwd`"/ps.sh"
chmod go-rwx,u+rx `pwd`"/ps.sh"
`pwd`"/ps.sh"

ssh -q -oStrictHostKeyChecking=no git@github.com exit
echo ssh check returned $?

while IFS= read archiveLine
do
  projectName=$(echo "$archiveLine" | cut -f 1 -d ",")
  projectVersion=$(echo "$archiveLine" | cut -f 2 -d ",")
  archiveLocation=$(echo "$archiveLine" | cut -f 3 -d ",")
  archiveFileName=$(echo "$archiveLocation" | sed 's|^.\+/\([^/]\+\)$|\1|')
	echo PROCESSING "$projectName" VERSION "$projectVersion"
	
	mkdir tempCompileDir
	pushd tempCompileDir > /dev/null
	
		wget "$archiveLocation"
	
		archiveSuccess=0
	
		#unzip archive
		tar -axf "$archiveFileName"
		#compile archive
		TEMP_COMPILE_DIR=`pwd`
	
	  tempFolder=$(ls | grep -E "$projectName-.+" | grep -vE "[.]tar")
	  echo "working folder: $tempFolder"
		pushd "$tempFolder" > /dev/null
		  CFLAGS=-fpermissive -Wno-error
		  CXXFLAGS=$CFLAGS
		  CPPFLAGS=$CFLAGS
		  if [ -f configure.ac -o -f Makefile.am -o -f configure.in -o -f Makefile.in -o -f configure ]
		  then
		    autotools_source=1
		    echo "DETECTED AUTOTOOLS"
		  else
		    if [ -f CMakeLists.txt ]
		    then
		      cmake_source=1
		    	echo "DETECTED CMAKE"
		    else
		    	echo "UNKOWN SOURCE TYPE, ASSUMING SIMPLY MAKE"
		    	echo "Directory listing:"
		    	ls -la
		    fi
		  fi
		  
		  if [ "$autotools_source" == "1" ]
		  then
				./configure > autobuild_configure1.1.log 2> autobuild_configure1.2.log
				tempRetval=$?
				if [ ! "$tempRetval" == "0" ]
				then
				  echo CONFIGURE FAILED
					cat autobuild_configure1.2.log
				else
					echo SUCCESS ON CONFIGURE
				fi
		  fi
		  if [ "$cmake_source" == "1" ]
		  then
				ccmake . > autobuild_ccmake.1.log 2> autobuild_ccmake.2.log
				tempRetval=$?
				if [ ! "$tempRetval" == "0" ]
				then
				  echo CCMAKE FAILED
					cat autobuild_ccmake.2.log
				else
					echo SUCCESS ON CCMAKE
				fi
		  fi
		  
			make > autobuild_make1.1.log 2> autobuild_make1.2.log
			tempRetval=$?
			if [ ! "$tempRetval" == "0" ]
			then
			  echo MAKE FAILED
				cat autobuild_make1.2.log
			  if [ "$autotools_source" == "1" ]
			  then
			  	echo TRYING AUTORECONF
			  	autoreconf -i -f > autobuild_autoreconf.1.log 2> autobuild_autoreconf.2.log
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo AUTORECONF FAILED
					  cat autobuild_autoreconf.2.log
					else
						echo SUCCESS ON AUTORECONF
					fi
					
			  	./configure > autobuild_configure2.1.log 2> autobuild_configure2.2.log
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo CONFIGURE STILL FAILED AFTER AUTORECONF
					  cat autobuild_configure2.2.log
					else
						echo SUCCESS ON SECOND CONFIGURE
					fi
					
			  	make > autobuild_make2.1.log 2> autobuild_make2.2.log
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo MAKE STILL FAILED AFTER AUTORECONF
					  cat autobuild_make2.2.log
					else
						echo SUCCESS ON SECOND MAKE
					fi
			  fi
			else
				echo SUCCESS ON FIRST MAKE
			fi
		popd > /dev/null
	
	popd > /dev/null
	rm -rf tempCompileDir
	
	if [ "$archiveSuccess" == "1" ]
	then
	  echo "$archiveLine" >> archives_successfully_processed
	else
	  echo "$archiveLine" >> archives_that_failed
	fi
done < archives_to_process
mv archives_that_failed archives_to_process

git config --get remote.origin.url
git remote set-url origin git@github.com:ryanniehaus/open_source_package_builder.git
git remote show origin
git checkout master
git add archives_to_process
git add archives_successfully_processed
git status
git commit -m "updating lists"
git push

