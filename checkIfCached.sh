# check list of files in a directory on /cache, and comapre
# to the list on tape

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [cache directory]"
  exit 1
fi
cachedirArg=$1

pushd $cachedirArg > /dev/null
cachedir=$(pwd -P | sed 's/^.*\/cache\//\/cache\//')
popd > /dev/null
tapedir=$(echo $cachedir|sed 's/^\/cache/\/mss/')

echo ""
echo "cachedir = $cachedir"
echo "tapedir = $tapedir"

mkdir -p tmp
ls $cachedir | grep -vi readme > tmp/cachelist
ls $tapedir | grep -vi readme > tmp/tapelist

echo ""
echo "DIFF:"
diff tmp/{cache,tape}list | tee tmp/difflist

echo ""
if [ -s tmp/difflist ]; then
  echo "WARNING: differences between cache and tape"
else
  echo "SUCCESS: no differences between cache and tape"
fi

