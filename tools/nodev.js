if(process.argv.length == 3)
{
   var varray = process.versions.v8.split(".");

   if(process.argv[2] == 'major') console.log(varray[0]);
   if(process.argv[2] == 'minor') console.log(varray[1]);
   if(process.argv[2] == 'patch') console.log(varray[2]);
   if(process.argv[2] == 'x') console.log(varray[3]);
}
