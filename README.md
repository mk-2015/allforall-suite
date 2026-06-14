# WARNING
- ***This project is still in development! Dont use in production!***
- ***I am not responsible to damages that occours if production is damaged!***

# All For All
- Is an opensource tool library for os-admins
- Is written in C++ for blazing fast applications and code readibility. So, other people can join.
- Is a massive toolkit (SOON...)

- build:
	* Support:
		- 1. Windows 
		- 2. Linux
		- 3. BSD
		- 4. MacOs (Never)
	* Requirements:
		- 1. internet
		- 2. gcc and make
	* steps:
		- step 1: clone the repo:
			```bash
			git clone --depth 1 https://github.com/mk-2015/allforall-suite
			```
		- step 2: cd and build:
			```bash
			cd allforall-suite && make deps install # Note make install executes make all before copying.
			```
		- step 3: reboot to guarantee max package changes:
			* linux/BSD:
			```bash
			sudo reboot
			```
			
			* windows:
			```cmd
			shutdown /r /t 0
			```
		- step 4: enjoy!
		
## alltools
- Binary tools for quick code writing in bash or batch
- Written in cpp

### Nothing tool
- The nothing tools does... as you guessed nothing
- The normal mode:
```bash
nothing
```
* Does nothing.
* Thats it, don't expect more.
	
- Sleeping mode:
```bash
nothing -s 1000 # Sleeps for 1 second or 1000 Milliseconds.
```
* Sleeps while doing nothing
* Thats it, again.
	
### Lockout tool
- Logout from a machine
- the normal mode:
```bash
lockout # Logout
```
* Logs out of the machine with specified API of the platform
* Cross-platform
- Sleep mode:
```bash
lockout -s 1000 # Sleep for 1Second or 1000ms
```
* Sleeps for specified time until logout

### Serve tool
- Create a server from a machine
- Start a server at 0.0.0.0:8080/ from the . folder
```bash
serve
```
* Serves files from 8080 from the . folder
```bash
serve --port 443 --cors --https --folder \"servethething\"
```
* serves from port 443
* adds CORS Headers
* uses https SSL
* serves from the servethething folder

### Cursor tool
- Also known as ```'spinner'``` makes a cool spinner for loading, but you don't know the time it will end at. Mouth full. 
- Bunch of flags:
	* -d or --d flag:
		* Requires a millisecond argument
		* Time to sleep before next character
	* --cmd "command to /run" flag:
		* Is required
		* Command to run until finished.
	* -th or --theme flag:
		* Is not required
		* Default theme is classic
		* Themes:
			- classic
			- num-wheel
			- ark
			- updot
	* -v or --verbose flag:
		* Makes everything verbose
	
	* -c or --copyright flag:
		* shows copyright.
		
### fwipe tool
- File wiper utility
- Flags:
	* -f or --file <FILE> Flag:
		- Required
		- Specifiy file to delete
		
	* -h or --help Flag:
		- Not Required
		- Bring up help menu
		
### dwipe tool
- Directory wiper utility 
- Flags:
	* -d or --dir Flag:
		- Required
		- Specifiy directory to delete
		
	* -h or -help Flag:
		- Not Required
		- Bring up help menu

## libdot3
- See '[libdot3 docs](./inner/libdot3/README.md)'