# consoleDropBox

Some simple console app for DropBox

## usage:

### upload file
  tool.exe put src dst  
  tool.exe put D:\file_to.upload /remote_folder/file_that_will_be_upploaded  


### download file
  tool.exe get src dst 
where src_path is a remote file  

### list for files
  tool.exe ls url  
where url is required

## remove files
  tool.exe rm url1 url2 ...  
where url is required  

## folder creation
  tool.exe mkdir url1 url2 ...  
where url is required  

### Other options
-h help message  
-v verbose  
-a token  
where token is your DropBox access token  
-k file  
where file name that contains access token  
Remote file is unix style   
/file  /folder/file


## third party libraries, dependency:
rapidjson 1.0.2 (1.0.3 not compiling)  
libcurl (curl 7.65.3)  
