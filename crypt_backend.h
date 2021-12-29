#ifndef CRYPT_BACKEND_H
#define CRYPT_BACKEND_H

#include <QString>
#include <QList>
#include <QCryptographicHash>
#include <QtNetwork/QNetworkInterface>

class crypt_backend {

private:
    /*!
     * \brief Hash algorithm to use. Determines length of service specific password.
     */
    inline static const QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256;

    /*!
     * \brief How many iterations to take when generating the password.
     */
    inline static const int iterations = 1000000;

    /*!
     * \brief Constant pepper for additional strength.
     */
    inline static const QString pepper = "M!cr0P4ssw0rdM4n4g3r";

public:
    /*!
     * \brief Charset from which to build the service-specific password. Do not change!
     */
    inline static const QString defaultCharset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_{|}~";

    /*!
     * \brief Generates service-specific password.
     * \param master    Master password string.
     * \param service   Service name string.
     * \param charset   Charset to build service specific password from.
     * \return Service specific password.
     */
    static QString generator(QString master, QString service, QString charset = defaultCharset);
    static QString generator(QString master, QString service) {
        return generator(master, service, defaultCharset);
    }

    /*!
     * \brief Combines and hashes list of byte-arrays.
     * \param data          List of byte-arrays.
     * \param iterations    Number of iterations.
     * \param algorithm     Hash algoritm to use.
     * \return Hashed byte-array.
     */
    static QByteArray* hasher(QList<QByteArray> data, int iterations, QCryptographicHash::Algorithm algorithm);

    /*!
     * \brief Generates system specific key based on hardware adress.
     * \return Byte-array of system specific key.
     */
    static QByteArray* getLocalKey();

    /*!
     * \brief Byte-wise encryption of array (modular block cipher).
     * \param data  Array of plain data.
     * \param key   Array of key.
     * \param m     Prime number. Must be greater than 256.
     * \return Encrypted array of data.
     */
    static QByteArray* encrypt(QByteArray data, QByteArray key, int m = 257);

    /*!
     * \brief Byte-wise decryption of array (modular block cipher).
     * \param data  Array of encrypted data.
     * \param key   Array of key.
     * \param m     Prime number. Must be greater than 256.
     * \return Plain array of data.
     */
    static QByteArray* decrypt(QByteArray data, QByteArray key, int m = 257);

    /*!
     * \brief Custom implementation of modulo.
     * \param n     Number.
     * \param m     Module.
     */
    static int mod(int n, int m);

    /*!
     * \brief Modular inverse of n in m.
     * \param n     Number.
     * \param m     Module.
     */
    static int modInv(int n, int m);
};

#endif // CRYPT_BACKEND_H
