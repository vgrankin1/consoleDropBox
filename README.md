# consoleDropBox

Some simple console app for DropBox

## usage:

### upload file
tool.exe put src_path dst_path  
where src_path is a local file  

### download file
tool.exe get src_path dst_path  
where src_path is a remote file  

### list for files
tool.exe ls url
where url is required

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
