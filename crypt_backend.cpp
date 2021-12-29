#include "crypt_backend.h"

QString crypt_backend::generator(QString master, QString service, QString charset) {

    // Convert strings to byte arrays
    QByteArray masterBytes = master.toUtf8();
    QByteArray serviceBytes = service.toUtf8();
    QByteArray pepperBytes = pepper.toUtf8();

    // Create hash
    QByteArray* buffer = hasher(QList<QByteArray>{masterBytes, serviceBytes, pepperBytes}, iterations, algorithm);

    // Use hash result to choose characters from char set
    QString password = "";
    for (int i = 0; i < buffer->length(); i += 2) {
        int index;
        if (charset == defaultCharset) {
            switch (i) { // This switch guarantees that every kind of character is in the password for very rare cases
                case 0: index = 62 + mod((buffer->at(i) << 8) + buffer->at(i+1), 30); break; // Get special character
                case 2: index = 36 + mod((buffer->at(i) << 8) + buffer->at(i+1), 26); break; // Get upper case character
                case 4: index = 10 + mod((buffer->at(i) << 8) + buffer->at(i+1), 26); break; // Get lower case character
                case 6: index = mod((buffer->at(i) << 8) + buffer->at(i+1), 10); break; // Get number
                default: index = mod((buffer->at(i) << 8) + buffer->at(i+1), charset.length()); // Get random character
            }
        } else {
            index = mod((buffer->at(i) << 8) + buffer->at(i+1), charset.length()); // Get random character
        }
        password.append(charset.at(index));
    }

    delete buffer;
    return password;
}

QByteArray* crypt_backend::hasher(QList<QByteArray> data, int iterations, QCryptographicHash::Algorithm algorithm) {

    // Setup hashing module and output buffer
    QCryptographicHash* hasher = new QCryptographicHash(algorithm);
    int numOutputBytes = QCryptographicHash::hashLength(algorithm);
    QByteArray* buffer = new QByteArray(numOutputBytes, 0);

    // Iterate to generate hash
    for (int i = 0; i < iterations; i++) {
        hasher->addData(*buffer);
        foreach (QByteArray chunk, data) {
            hasher->addData(chunk);
        }
        buffer->replace(0, numOutputBytes, hasher->result());
        hasher->reset();
    }

    delete hasher;
    return buffer;
}

QByteArray* crypt_backend::getLocalKey() {

    // Obtain mac adress
    QString macAdress;
    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack))
            macAdress = interface.hardwareAddress();
    }

    // Reshape string to byte array
    QStringList macParts = macAdress.split(':', QString::SplitBehavior::SkipEmptyParts);
    QByteArray* macBytes = new QByteArray();
    foreach (QString part, macParts) {
        bool ok;
        macBytes->append(part.toInt(&ok, 16));
    }

    // Generate key
    QByteArray* key = hasher(QList<QByteArray>{*macBytes}, macBytes->at(0), algorithm);
    delete macBytes;
    return key;
}

QByteArray* crypt_backend::encrypt(QByteArray data, QByteArray key, int m) {
    QByteArray* encrypted = new QByteArray();
    int multiplier, dataPoint;
    for (int i = 0; i < data.length(); i++) {
        multiplier = mod(key.at(mod(i, key.length())), 256);
        dataPoint = mod(data.at(i), 256);
        encrypted->append(mod(dataPoint * multiplier, m));
    }
    return encrypted;
}

QByteArray* crypt_backend::decrypt(QByteArray data, QByteArray key, int m) {
    QByteArray* decrypted = new QByteArray();
    int inverse, dataPoint;
    for (int i = 0; i < data.length(); i++) {
        inverse = mod(modInv(mod(key.at(mod(i, key.length())), 256), m), 256);
        dataPoint = mod(data.at(i), 256);
        decrypted->append(mod(dataPoint * inverse, m));
    }
    return decrypted;
}

int crypt_backend::mod(int n, int m) {
    int result = n % m;
    while (result < 0)
        result += m;
    return result;
}

int crypt_backend::modInv(int n, int m) {
    int b0 = m, t, q;
    int x0 = 0, x1 = 1;
    if (m == 1)
        return 1;
    while (n > 1) {
        q = n / m;
        t = m, m = mod(n, m), n = t;
        t = x0, x0 = x1 - q * x0, x1 = t;
    }
    if (x1 < 0)
        x1 += b0;
    return x1;
}

