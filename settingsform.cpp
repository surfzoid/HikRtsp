#include "settingsform.h"
#include "ui_settingsform.h"


SimpleCrypt SettingsForm::crypto;
SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
    QSettings settings;


    crypto.setKey(Q_UINT64_C(0x0c2ad4a4acb9f023 * 3));//some random number
    settings.beginGroup("CamsName");
    QStringList keys = settings.allKeys();

    ui->CamNameEd->clear();
    for (int i=0; i<keys.length(); i++){
        ui->CamNameEd->addItem(keys.at(i));
    }

    settings.endGroup();

    ui->ConfFsPath->setText(settings.fileName());
}

SettingsForm::~SettingsForm()
{
    delete ui;
}

void SettingsForm::on_BtnSave_released()
{
    QSettings settings;
    CamName = ui->CamNameEd->currentText();
    settings.setValue( "CamsName/" + CamName,"Name");
    //settings.endGroup();
    settings.sync();

    settings.beginGroup(CamName);
    // settings.group() == "alpha/beta"

    settings.setValue( "Ip", ui->IpEd->text());
    settings.setValue("Port", ui->PortEd->text());
    settings.setValue("PortHttp", ui->PortHttpEd->text());
    settings.setValue("User", ui->UserEd->text());
    settings.setValue("Password",crypto.encryptToString(ui->PassEd->text()) );

    settings.endGroup();
    settings.sync();
    close();
}

void SettingsForm::on_CamNameEd_currentIndexChanged(const QString &arg1)
{
    QSettings settings;
    CamName = arg1;
    settings.beginGroup(CamName);

    CamIp = settings.value("Ip", "").value<QString>();
    CamPort = settings.value("Port", "554").value<QString>();
    CamPortHttp = settings.value("PortHttp", "800").value<QString>();
    CamUser = settings.value("User", "admin").value<QString>();
    CamPass = crypto.decryptToString(settings.value("Password", "hik12345").value<QString>());
    settings.endGroup();

    ui->IpEd->setText(CamIp);
    ui->PortEd->setText(CamPort);
    ui->PortHttpEd->setText(CamPortHttp);
    ui->UserEd->setText(CamUser);
    ui->PassEd->setText(CamPass);

}

void SettingsForm::on_BtnDel_released()
{
    QSettings settings;
    settings.remove(CamName);
    settings.remove("CamsName/" + CamName);
    settings.sync();
    ui->CamNameEd->removeItem(ui->CamNameEd->currentIndex());
}

void SettingsForm::on_BtPlay_released()
{

}

void SettingsForm::on_BtnCancel_released()
{
    close();
}

QStringList SettingsForm::FillCmbUris()
{
    QSettings settings;
    QStringList Uris;
    settings.beginGroup("CamsName");
    QStringList keys = settings.allKeys();
    settings.endGroup();

    for (int i=0; i<keys.length(); i++){

        CamName = keys.at(i);
        settings.beginGroup(CamName);

        CamIp = settings.value("Ip", "").value<QString>();
        CamPort = settings.value("Port", "554").value<QString>();
        CamPortHttp = settings.value("PortHttp", "800").value<QString>();
        CamUser = settings.value("User", "admin").value<QString>();
        CamPass = crypto.decryptToString(settings.value("Password", "hik12345").value<QString>());
        settings.endGroup();

        if (CamPass != "hik12345") {
            Uris.append( "rtsp://" + CamUser + ":" + CamPass + "@" + CamIp + ":" + CamPort + "/ISAPI/streaming/channels/");
        }else
        {
            Uris.append( "rtsp://" + CamIp + ":" + CamPort + "/ISAPI/streaming/channels/");
        }
    }

    return Uris;
}
