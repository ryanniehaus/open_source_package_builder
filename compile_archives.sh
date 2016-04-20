#!/usr/bin/env bash

urlencode() {
    # urlencode <string>
    old_lc_collate=$LC_COLLATE
    LC_COLLATE=C
    
    local length="${#1}"
    for (( i = 0; i < length; i++ )); do
        local c="${1:i:1}"
        case $c in
            [a-zA-Z0-9.~_-]) printf "$c" ;;
            *) printf '%%%02X' "'$c" ;;
        esac
    done
    
    LC_COLLATE=$old_lc_collate
}

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

git config --get remote.origin.url
git remote set-url origin git@github.com:ryanniehaus/open_source_package_builder.git
git remote show origin
git remote set-branches --add origin master
git remote show origin
git fetch
git pull

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
	projectIsPreReleaseString="false"
	checkForProjectPreReleaseIndicator=$(echo "$projectVersion" | grep -E "beta|alpha|[Rr][Cc]")
	currentArch=$(arch)
	
	if [ "$currentArch" == "x86_64" ]
	then
	  currentArch="amd64"
	fi
	
	if [ ! "$checkForProjectPreReleaseIndicator" == "" ]
	then
	  projectIsPreReleaseString="true"
	fi
	
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
	  cp -rf $tempFolder $tempFolder.working
	  mv $tempFolder ../$tempFolder
	  echo "working folder: $tempFolder.working"
		pushd "$tempFolder.working" > /dev/null
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
				./configure
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
		  
			make -s
			tempRetval=$?
			if [ ! "$tempRetval" == "0" ]
			then
			  echo MAKE FAILED
				echo CLEANING UP FROM LAST BUILD
				make -s clean
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
					
			  	./configure
					tempRetval=$?
					if [ ! "$tempRetval" == "0" ]
					then
					  echo CONFIGURE STILL FAILED AFTER AUTORECONF
					else
						echo SUCCESS ON SECOND CONFIGURE
					fi
					
			  	make -s
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
			
			if [ "$archiveSuccess" == "1" ]
			then
			  archiveSuccess=0
			  
			  if [ ! -d doc-pak ]
			  then
			    mkdir -v doc-pak
  				cp -v *ABOUT* *README* *INSTALL* *COPYING* *LICENSE* *RELEASE* *VERSION* *NEWS* *PROVENANCE* *Changelog* *TODO* *CREDITS* doc-pak/
			  fi
			  
			  echo "$projectName version $projectVersion" > description-pak
			  echo >> description-pak
			  cat *ABOUT* *README* *INSTALL* *LICENSE* | head -n 9 >> description-pak
			  
			  licenseTitle=$(cat *LICENSE* | grep -vE "^$" | head -n 1)
			  checkInstallCommonOptions=$( echo --install=no \
			  	--fstrans=yes \
			  	--pkgname="$projectName" \
			  	--pkgversion="$projectVersion" \
			  	--pkgarch="$currentArch" \
			  	--pkgrelease="1+ryryautobuild" \
			  	--pkgsource="$archiveLocation" \
			  	--pkgaltsource="https://github.com/ryanniehaus/open_source_package_builder/releases/download/$projectName-$projectVersion/$archiveFileName" \
			  	--pakdir="$(pwd)/.." \
					--maintainer="ryan.niehaus@gmail.com" \
					-y)
				# --pkglicense="$licenseTitle"
			  sudo checkinstall -S $checkInstallCommonOptions make -s install
				if [ "$tempRetval" == "0" ]
				then
			  	archiveSuccess=1
				fi
				
				sudo urpmi rpm-build
			  #sudo checkinstall -R $checkInstallCommonOptions make -s install
				if [ "$tempRetval" == "0" ]
				then
			  	archiveSuccess=1
				fi
				
			  sudo checkinstall -D $checkInstallCommonOptions make -s install
				if [ "$tempRetval" == "0" ]
				then
			  	archiveSuccess=1
				fi
				ls -la ..
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
		  echo "  \"prerelease\": $projectIsPreReleaseString" >> releaseCreationRequest.json
		  echo "}" >> releaseCreationRequest.json
		  
		  curl -u "ryanniehaus:$GITHUB_PERSONAL_ACCESS_TOKEN" -X POST -d "$(cat releaseCreationRequest.json)" --header "Accept: application/vnd.github.v3+json" "https://api.github.com/repos/ryanniehaus/open_source_package_builder/releases" > new_release_response.log
		  curlRetval="$?"
		  if [ "$curlRetval" == "0" ]
		  then
				uploadBaseURL=$(cat new_release_response.log | dos2unix | grep -E "^[[:space:]]+[\"]upload_url[\"]:[[:space:]]+[\"][^\"]+[\"],$" | sed 's|^[[:space:]]*[\"]upload_url[\"]:[[:space:]]*[\"]\([^\"]*\)[\"],$|\1|;s|{[^}]\+}$||')
				
				for eachArchive in $(ls "$archiveFileName" *.tgz *.deb *.rpm)
				do
				  echo UPLOADING "$eachArchive"
				  
				  contentType=$(file -b --mime-type "$eachArchive")
				  urlEncodedLabel=$(urlencode "$eachArchive")
				  
					curl -1 -X POST -u "ryanniehaus:$GITHUB_PERSONAL_ACCESS_TOKEN" --data-binary "@$eachArchive"  --header "Accept: application/vnd.github.v3+json" --header "Content-Type: $contentType" "$uploadBaseURL?name=$eachArchive&label=$urlEncodedLabel"
					curlRetval="$?"
					if [ "$curlRetval" == "0" ]
					then
						archiveSuccess=1
					fi
				done
		  fi
		fi
	
	popd > /dev/null
	rm -rf tempCompileDir
	
	if [ "$archiveSuccess" == "1" ]
	then
	  echo "$archiveLine" >> NEWarchives_successfully_processed
	else
	  echo "$archiveLine" >> archives_that_failed
	  git add $tempFolder
	  git commit -m "adding source folder to failed compile branch"
	  git fetch && git pull && git push
	  git fetch && git pull && git push
	  git fetch && git pull && git push
	fi
	fi
done < archives_to_process

git checkout master
git pull

cat archives_successfully_processed >> NEWarchives_successfully_processed
sort -t"," -k1,1d -k2,2V -u NEWarchives_successfully_processed | grep -vE "^$" > archives_successfully_processed
git add archives_successfully_processed
git status
git commit -m "updating lists"
git fetch && git pull && git push
git fetch && git pull && git push
git fetch && git pull && git push

