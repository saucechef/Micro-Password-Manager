## Micro Password Manager

Lightweight local password manager that generates passwords by hashing a master password salted with service names.

Also provides convenience features like reading list of services and usernames from a file and saving an encrypted version of the master password.

Read more about setup and usage in MANUAL.txt or ANLEITUNG.txt (german).

### Security

Password generation algorithm is protected against brute-forcing by repeated hashing.

Master password is encrypted with byte-wise modular block chiffre when saved.

### Benefits

* Users only need to memorize one strong password.
* Generates strong and unique passwords for every service.
* Generally quicker to use than manual login.
* Does not save any password (other than the master password, which is optional).

### Drawbacks

* Laborious to set up because service passwords have to be changed to generated passwords.
* Loss of the master password is fatal, but can only occur through user error.
