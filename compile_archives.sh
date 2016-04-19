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

> archives_that_failed
> NEWarchives_successfully_processed
while IFS= read archiveLine
do
  projectName=$(echo "$archiveLine" | cut -f 1 -d ",")
  projectVersion=$(echo "$archiveLine" | cut -f 2 -d ",")
  archiveLocation=$(echo "$archiveLine" | cut -f 3 -d ",")
  archiveFileName=$(echo "$archiveLocation" | sed 's|^.\+/\([^/]\+\)$|\1|')
	projectNameAndVersion=$(echo "$archiveLine" | sed 's|^\([^,]\+\),\([^,]\+\),\([^,]\+\)$|\1,\2,|')
	alreadyProcessed=$(grep -E "^$projectNameAndVersion" archives_successfully_processed)
	
	if [ ! "$alreadyProcessed" == "" ]
	then
		echo "$projectName VERSION $projectVersion ALREADY SUCCESSFULLY PROCESSED, SKIPPING..."
	else
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
		  CFLAGS=
		  CXXFLAGS="-fpermissive -Wno-error"
		  CPPFLAGS="$CXXFLAGS"
		  export CFLAGS CXXFLAGS CPPFLAGS
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
		    #./configure --help
				./configure CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" CPPFLAGS="$CPPFLAGS"
				tempRetval=$?
				if [ ! "$tempRetval" == "0" ]
				then
				  echo CONFIGURE FAILED
				else
					echo SUCCESS ON CONFIGURE
				fi
		  fi
		  if [ "$cmake_source" == "1" ]
		  then
				ccmake .
				tempRetval=$?
				if [ ! "$tempRetval" == "0" ]
				then
				  echo CCMAKE FAILED
				else
					echo SUCCESS ON CCMAKE
				fi
		  fi
		  
			make
			tempRetval=$?
			if [ ! "$tempRetval" == "0" ]
			then
			  echo MAKE FAILED
				echo CLEANING UP FROM LAST BUILD
				make clean
			  if [ "$autotools_source" == "1" ]
			  then
			  	echo TRYING AUTORECONF
			  	autoreconf -i -f
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo AUTORECONF FAILED
					else
						echo SUCCESS ON AUTORECONF
					fi
					
			  	./configure CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" CPPFLAGS="$CPPFLAGS"
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo CONFIGURE STILL FAILED AFTER AUTORECONF
					else
						echo SUCCESS ON SECOND CONFIGURE
					fi
					
			  	make
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo MAKE STILL FAILED AFTER AUTORECONF
					else
						echo SUCCESS ON SECOND MAKE
						archiveSuccess=1
					fi
			  fi
			else
				echo SUCCESS ON FIRST MAKE
				archiveSuccess=1
			fi
		popd > /dev/null
		
		if [ "$archiveSuccess" == "1" ]
		then
		  archiveSuccess=0
		  echo "{" > releaseCreationRequest.json
		  echo "  \"tag_name\": \"$projectName-$projectVersion\"," >> releaseCreationRequest.json
		  echo "  \"target_commitish\": \"master\"," >> releaseCreationRequest.json
		  echo "  \"name\": \"$projectName-$projectVersion\"," >> releaseCreationRequest.json
		  echo "  \"body\": \"Autobuilt version of $projectName-$projectVersion, downloaded from $archiveLocation\"," >> releaseCreationRequest.json
		  echo "  \"draft\": false," >> releaseCreationRequest.json
		  echo "  \"prerelease\": false" >> releaseCreationRequest.json
		  echo "}" >> releaseCreationRequest.json
		  
		  curl -u "ryanniehaus:$GITHUB_PERSONAL_ACCESS_TOKEN" -i -v -X POST -d "$(cat releaseCreationRequest.json)" --header "Accept: application/vnd.github.v3+json" "https://api.github.com/repos/ryanniehaus/open_source_package_builder/releases" > new_release_response.log
		  curlRetval="$?"
		  if [ "$curlRetval" == "0" ]
		  then
				uploadBaseURL=$(cat new_release_response.log | dos2unix | grep -E "^[[:space:]]+[\"]upload_url[\"]:[[:space:]]+[\"][^\"]+[\"],$" | sed 's|^[[:space:]]*[\"]upload_url[\"]:[[:space:]]*[\"]\([^\"]*\)[\"],$|\1|;s|{[^}]\+}$||')
				
				curl -1 -i -v -X POST -u "ryanniehaus:$GITHUB_PERSONAL_ACCESS_TOKEN" --data-binary "@$archiveFileName"  --header "Accept: application/vnd.github.v3+json" --header "Content-Type: application/gzip" "$uploadBaseURL?name=$archiveFileName&label=archive%20of%20source%20used%20to%20create%20build%20for%20$projectName%20v$projectVersion"
				curlRetval="$?"
				if [ "$curlRetval" == "0" ]
				then
				  archiveSuccess=1
				fi
		  fi
		fi
	
	popd > /dev/null
	rm -rf tempCompileDir
	
	if [ "$archiveSuccess" == "1" ]
	then
	  echo "$archiveLine" >> NEWarchives_successfully_processed
	else
	  echo "$archiveLine" >> archives_that_failed
	fi
	fi
done < archives_to_process

git config --get remote.origin.url
git remote set-url origin git@github.com:ryanniehaus/open_source_package_builder.git
git remote show origin
git remote set-branches --add origin master
git fetch
git checkout master
git pull
cat NEWarchives_successfully_processed >> archives_successfully_processed
git add archives_successfully_processed
git status
git commit -m "updating lists"
git push

