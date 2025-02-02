#include "chatwidget.h"

#include <QMainWindow>

ChatWidget::ChatWidget(QMainWindow *MIOWindow)
{
    /// On charge l'UI
    setupUi(this);

    /// On créé un Socket qui représente la connection au serveur
    m_socket = new QTcpSocket(this);

    /// On créé les connections entre le socket et nos slots
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));
    /// On initialise la taille de message à 0
    m_tailleMessage = 0;
    //connect(MIOWindow->getSettingsWindow->getOkbutton, SIGNAL(clicked()))
}

/**
 * @brief ChatWidget::on_boutonConnexion_clicked Slot pour une tentative de connexion au serveur
 */
void ChatWidget::on_boutonConnexion_clicked()
{
    /// On annonce sur la fenêtre qu'on est en train de se connecter
    listeMessages->append("<em>Tentative de connexion en cours...</em>");

    /// On désactive les connexions précédentes s'il y en a
    m_socket->abort();

    /// On se connecte sur le serveur
    m_socket->connectToHost(serveurIP->text(), serveurPort->value());
}


/**
 * @brief ChatWidget::on_boutonEnvoyer_clicked Slot pour envoyer le message
 */
void ChatWidget::on_boutonEnvoyer_clicked()
{
    /// On créé un array d'octets
    QByteArray paquet;

    QDataStream out(&paquet, QIODevice::WriteOnly);

    /// On prépare le paquet à envoyer
    QString messageAEnvoyer = "<strong>" + pseudo->text() +"</strong> : " + message->text();

    /// On met temporairement 0 pour la taille du message
    out << (quint16) 0;
    /// On met le message à envoyer
    out << messageAEnvoyer;

    /// On revient en position 0 dans le data stream
    out.device()->seek(0);

    /// On y met la taille du paquet
    out << (quint16) (paquet.size() - sizeof(quint16));

    /// On envoie le paquet
    m_socket->write(paquet);

    /// On vide la zone d'écriture du message
    message->clear();

    /// On met le curseur à l'intérieur
    message->setFocus();
}

/**
 * @brief ChatWidget::on_message_returnPressed Slot lorsequ'on appuise sur la touche entrée du chat
 */
void ChatWidget::on_message_returnPressed()
{
    on_boutonEnvoyer_clicked();
}

/**
 * @brief ChatWidget::donneesRecues Slot activé quand on recoit un paquet, pareil que sur serveur, mais avec affichage dans une fenêtre de texte
 */
void ChatWidget::donneesRecues()
{
    /// Socket du serveur
    QDataStream in(m_socket);

    /// On regarde si on connait la taille du message
    if (m_tailleMessage == 0)
    {
        /// Si le maquet n'est pas complet, on quitte
        if (m_socket->bytesAvailable() < (int)sizeof(quint16))
             return;
        /// On met dans m_tailleMessage la taille du message
        in >> m_tailleMessage;
    }

    /// On quitte si la taille du paquet ne corresponds pas à l'entête
    if (m_socket->bytesAvailable() < m_tailleMessage)
        return;


    /// On écrit dans messageRecu le contenu du message
    QString messageRecu;
    in >> messageRecu;

    /// On affiche le message sur la zone de Chat
    listeMessages->append(messageRecu);

    /// On remet la taille de m_tailleMessage à 0 pour pouvoir recevoir de futurs messages
    m_tailleMessage = 0;
}

/**
 * @brief ChatWidget::connecte Slot est appelé lorsque la connexion au serveur a réussi
 */
void ChatWidget::connecte()
{
    listeMessages->append("<em>Connexion réussie !</em>");
}

/**
 * @brief ChatWidget::deconnecte Slot appelé lorsqu'on est déconnecté du serveur
 */
void ChatWidget::deconnecte()
{
    listeMessages->append("<em>Déconnecté du serveur</em>");
}

/**
 * @brief ChatWidget::erreurSocket Slot appelé lorsqu'il y a une erreur
 * @param erreur
 */
void ChatWidget::erreurSocket(QAbstractSocket::SocketError erreur)
{
    /// On affiche un message différent selon l'erreur qu'on nous indique
    switch(erreur)
    {
        case QAbstractSocket::HostNotFoundError:
            listeMessages->append("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            listeMessages->append("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            listeMessages->append("<em>ERREUR : le serveur a coupé la connexion.</em>");
            break;
        default:
            listeMessages->append("<em>ERREUR : " + m_socket->errorString() + "</em>");
    }

    boutonConnexion->setEnabled(true);
}

ChatWidget::~ChatWidget()
{

}
