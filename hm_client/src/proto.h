/*
   hm_client -  hearthmod client
   Copyright (C) 2016 Filip Pancik

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PROTO_H
#define PROTO_H


enum packet_e {
    P_NONE = 0,
    P_CREATEACCOUNT = 1,
    P_LOGIN = 2,
    P_DECK = 3,
};

namespace Ui {
class Proto;
}

class Proto
{

public:
    explicit Proto();
    ~Proto();

    int add(char **dst, const char *maxdst, const void *src, const int nsrc);
    int packet(char *dst, enum packet_e packet, char *src, const int nsrc);
    enum packet_e read(char *src, const int nsrc);
/*
private slots:
    void secureConnect();
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketEncrypted();
    void sslErrors(const QList<QSslError> &errors);
    void socketError(QAbstractSocket::SocketError);
    void socketReadyRead();
    void updateEnabledState();

    void addCard();
    void clearDeck();
    void removeCard();
    void saveDeck();

private:
    void setNcards(const int type);

    int ncards;
    Ui::MainWindow *ui;
    QSslSocket *socket;
*/

};

#endif // PROTO_H
