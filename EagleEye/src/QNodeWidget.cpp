#include "QNodeWidget.h"

#include "ui_QNodeWidget.h"
#include <boost/bind.hpp>
#include <QPalette>


IQNodeInterop::IQNodeInterop(boost::shared_ptr<EagleLib::Parameter> parameter_, QWidget* parent, EagleLib::Node* node_) :
    QWidget(parent),
    node(node_)
{
    layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    nameElement = new QLabel(QString::fromStdString(parameter_->name), parent);
    nameElement->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Fixed);
    proxy = dispatchParameter(this, parameter_, node_);
    if (proxy)
    {
        layout->addWidget(proxy->getWidget(), 0, 1);
    }
    layout->addWidget(nameElement, 0, 0);
    nameElement->setToolTip(QString::fromStdString(parameter_->toolTip));
    parameter_->updateCallback = boost::bind(&IQNodeInterop::onParameterUpdate,this, _1);
    layout->addWidget(new QLabel(QString::fromStdString(type_info::demangle(parameter_->typeInfo.name()))), 0,2);
}

IQNodeInterop::~IQNodeInterop()
{
    delete proxy;
}

void IQNodeInterop::updateUi()
{
    if (proxy)
        proxy->updateUi();
}

void IQNodeInterop::on_valueChanged(double value)
{
    if (proxy)
        proxy->onUiUpdated();
}

void IQNodeInterop::on_valueChanged(int value)
{
    if (proxy)
        proxy->onUiUpdated();
}

void IQNodeInterop::on_valueChanged(bool value)
{
    if (proxy)
        proxy->onUiUpdated();
}
void IQNodeInterop::on_valueChanged(QString value)
{
    if (proxy)
        proxy->onUiUpdated();
}

void IQNodeInterop::on_valueChanged()
{
    if (proxy)
        proxy->onUiUpdated();
}
void IQNodeInterop::onParameterUpdate(boost::shared_ptr<EagleLib::Parameter> parameter)
{
    updateUi();
}

QNodeWidget::QNodeWidget(QWidget* parent, EagleLib::Node* node) :
    QWidget(parent),
	ui(new Ui::QNodeWidget())
{
	ui->setupUi(this);
    statusDisplay = new QLineEdit();
    warningDisplay = new QLineEdit();
    errorDisplay = new QLineEdit();
    criticalDisplay = new QLineEdit();
    ui->verticalLayout->addWidget(statusDisplay);
    ui->verticalLayout->addWidget(warningDisplay);
    ui->verticalLayout->addWidget(errorDisplay);
    ui->verticalLayout->addWidget(criticalDisplay);
    statusDisplay->hide();
    warningDisplay->hide();
    errorDisplay->hide();
    criticalDisplay->hide();
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    node->statusCallback    = boost::bind(&QNodeWidget::on_status, this, _1, _2);
    node->warningCallback   = boost::bind(&QNodeWidget::on_warning, this, _1, _2);
    node->errorCallback     = boost::bind(&QNodeWidget::on_error, this, _1, _2);

	if (node)
	{
        ui->chkEnabled->setChecked(node->enabled);
        connect(ui->chkEnabled, SIGNAL(clicked(bool)), this, SLOT(on_enableClicked(bool)));
		nodeId = node->GetObjectId();
		ui->nodeName->setText(QString::fromStdString(node->fullTreeName));
        ui->verticalLayout->setSpacing(0);
		for (int i = 0; i < node->parameters.size(); ++i)
		{
            auto interop = new IQNodeInterop(node->parameters[i], this, node);
            //interop->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
            ui->verticalLayout->addWidget(interop);
		}
	}
}
QNodeWidget::~QNodeWidget()
{}

void QNodeWidget::on_enableClicked(bool state)
{    EagleLib::NodeManager::getInstance().getNode(nodeId)->enabled = state;     }

EagleLib::Node* QNodeWidget::getNode()
{
	if (nodeId.IsValid())
	{
		return EagleLib::NodeManager::getInstance().getNode(nodeId);
	}
	return nullptr;
}
void QNodeWidget::on_status(const std::string& msg, EagleLib::Node* node)
{
    statusDisplay->show();
    statusDisplay->setText("Status: " + QString::fromStdString(msg));
}

void QNodeWidget::on_warning(const std::string& msg, EagleLib::Node* node)
{
    warningDisplay->show();
    warningDisplay->setText("Warning: " + QString::fromStdString(msg));
}

void QNodeWidget::on_error(const std::string& msg, EagleLib::Node* node)
{
    errorDisplay->show();
    errorDisplay->setText("Error: " + QString::fromStdString(msg));
}

void QNodeWidget::on_critical(const std::string& msg, EagleLib::Node* node)
{
    criticalDisplay->show();
    criticalDisplay->setText("Critical: " + QString::fromStdString(msg));
}
void QNodeWidget::setSelected(bool state)
{
    QPalette pal(palette());
    if(state == true)
        pal.setColor(QPalette::Background, Qt::green);
    else
        pal.setColor(QPalette::Background, Qt::gray);
    setAutoFillBackground(true);
    setPalette(pal);
}
QInputProxy::QInputProxy(IQNodeInterop* parent, boost::shared_ptr<EagleLib::Parameter> parameter_, EagleLib::Node* node_):
    node(node_)
{
    box = new QComboBox(parent);
    parameter = parameter_;
    updateUi();
    parent->connect(box, SIGNAL(currentIndexChanged(int)), parent, SLOT(on_valueChanged(int)));
}

void QInputProxy::onUiUpdated()
{
    QString inputName = box->currentText();
    parameter->setSource(inputName.toStdString());
    //node->setInputParameter(inputName.toStdString(), parameter->name);
}
QWidget* QInputProxy::getWidget()
{
    return box;
}
void QInputProxy::updateUi()
{
    box->clear();
    auto inputs = node->findCompatibleInputs(parameter->name);
    for(int i = 0; i < inputs.size(); ++i)
    {
        box->addItem(QString::fromStdString(inputs[i]));
    }
}

template<typename T> bool acceptsType(Loki::TypeInfo& type)
{
    return Loki::TypeInfo(typeid(T)) == type || Loki::TypeInfo(typeid(T*)) == type || Loki::TypeInfo(typeid(T&)) == type;
}
#define MAKE_TYPE(type) if(acceptsType<type>(parameter->typeInfo)) return new QNodeProxy<type, false>(parent, parameter);
#define MAKE_TYPE_(type) if(acceptsType<type>(parameter->typeInfo)) return new QNodeProxy<type, true>(parent, parameter);


IQNodeProxy* dispatchParameter(IQNodeInterop* parent, boost::shared_ptr<EagleLib::Parameter> parameter, EagleLib::Node *node)
{
    if(parameter->type & EagleLib::Parameter::Input)
    {
        // Create a ui for finding relavent inputs
        return new QInputProxy(parent, parameter, node);
    }
    if(parameter->type & EagleLib::Parameter::Control)
    {
        MAKE_TYPE(double);
        MAKE_TYPE(float);
        MAKE_TYPE(int);
        MAKE_TYPE(unsigned int);
        MAKE_TYPE(short);
        MAKE_TYPE(unsigned short);
        MAKE_TYPE(char);
        MAKE_TYPE(unsigned char);
        MAKE_TYPE(std::string);
        MAKE_TYPE(boost::filesystem::path);
        MAKE_TYPE(bool);
        MAKE_TYPE(boost::function<void(void)>);
    }

    if(parameter->type & EagleLib::Parameter::Output || parameter->type & EagleLib::Parameter::State)
    {
        MAKE_TYPE_(double);
        MAKE_TYPE_(float);
        MAKE_TYPE_(int);
        MAKE_TYPE_(unsigned int);
        MAKE_TYPE_(short);
        MAKE_TYPE_(unsigned short);
        MAKE_TYPE_(char);
        MAKE_TYPE_(unsigned char);
        MAKE_TYPE_(std::string);
        MAKE_TYPE_(boost::filesystem::path);
        MAKE_TYPE_(bool);
    }


	return nullptr;
}


