BEGIN {
  FS=" "
  needdata=0
  oldtime=""
  indoor_count=0
  indoor_sum=0
  date=""
}

function hex2dec(h,i,x,v){
  h=tolower(h);sub(/^0x/,"",h)
  for(i=1;i<=length(h);++i){
    x=index("0123456789abcdef",substr(h,i,1))
    if(!x)return "NaN"
    v=(16*v)+x-1
  }
  return v
}

/405]/ {
  if(needdata) {
    num=indoor_sum / (indoor_count * 10)
    printf(",%s,%s\n", num, date)
    needdata=0
    indoor_count=0
    indoor_sum=0
  }


  newtime=substr($2,1,5)
  minten=substr(newtime,5,1)
#  if(newtime != oldtime && (minten == "0" || minten == "3")) {
#  if(newtime != oldtime && (minten == "0")) {
  if(newtime != oldtime && (minten == "0" || minten == "5")) {
    num=hex2dec(substr($0,39,4))
    if(num > 60000)
      num=num - 65536
    num=num / 10
    printf("%s,%s", substr($2,1,5), num)
    needdata=1
    oldtime=newtime
    date=$1
  }
}

/404]/ {
  num=hex2dec(substr($0,39,4))
  indoor_sum=indoor_sum + num
  indoor_count=indoor_count + 1

#  if(needdata && indoor_count >= 70) {
#    num=indoor_sum / (indoor_count * 10)
#    printf(",%s\n", num)
#    needdata=0
#    indoor_count=0
#    indoor_sum=0
#  }
}

END {
  if(needdata) {
    num=indoor_sum / (indoor_count * 10)
    printf(",%s\n", num)
    needdata=0
    indoor_count=0
    indoor_sum=0
  }
}

#2010-09-07 06:53:36.321 [  405]   (8) 005A11C6FFFF1101
