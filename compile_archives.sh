#!/usr/bin/env bash

wget https://github.com/ryanniehaus/heroku-bash-buildpack/raw/master/bin/dataURLFromCloudinary.py
chmod u+rx dataURLFromCloudinary.py

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
	echo PROCESSING "$projectName" VERSION "$projectVersion"
	wget "$archiveLocation"
done < archives_to_process


git add archives_to_process
git commit -m "updating lists"

