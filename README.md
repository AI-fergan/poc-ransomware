# Ransomware-poc

This project contains a basic poc of a ransom type malware that demonstrates
the actions that ransom type malware performs when it activated in the system,
the malware generates the key and uses it to encrypt all the files in its folder (except the malware itself),
then it sends the key to the server and then deletes it from the machine on it activated
and waiting for input of the key from the user for decryption.


## Run Locally
- Compile the malware and the server and run server, you can use gcc & g++ to do that.

```bash
  gcc -o ransom ransom.c
```
```bash
  g++ -o server server.cpp
  ./server
```

- Then copy the malware to another folder and run it, After it finishes the encrypting you will see the ransom dialog.
- Now you can go back to the server and copy the key to the ransom dialog and decrypt the files.

## Warning
```diff
Please be careful when running this malware!
Run it only in the sample folders you create to test this malware.
```
