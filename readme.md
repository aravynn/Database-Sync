Hose Control Sync
-- 

Created by Kevin Jones for MEP Brothers Ltd.
Using a command line interface only.  

-- 

Summary: The Hose Control Sync program is a companion project to the Hose
Control App. This application is made for a cron job to run the application at 
night when the Hose Control application is not in use.  

--

Project includes: The project connects to the Hose Control database, and uses cURL
to connect to a PHP-based application online, to transfer the data securely to an
online databse.

This software is designed to only work on windows, as it is used as an extension of 
the Hose Control Software.

-- 

Status and To-Do: This project is considered completed and is running 
version 1.0. This software has been running effectively for approxiately 
1 year. This program will likely become obsolete once the Hose Control program is 
updated to use mySQL, and no longer relies on a local database.

To-Do: This program requires a small update to adapt to minor issues with data 
formatting to deal with backslashes in inconsistent cases online. This bug is 
currently the only significant issue. 