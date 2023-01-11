#include "myglwidget.h"
#include "ui_myglwidget.h"

MyGLWidget::MyGLWidget(QWidget *parent) :
    QOpenGLWidget(parent),m_program(0), m_shader(0), m_texture(0),
    ui(new Ui::MyGLWidget)
{
    ui->setupUi(this);
}

MyGLWidget::~MyGLWidget()
{
    // Make sure the context is current and then explicitly
    // destroy all underlying OpenGL resources.
    //makeCurrent();

    delete m_texture;
    delete m_shader;
    delete m_program;

    m_vbo.destroy();
    m_vao.destroy();

    //doneCurrent();
    delete ui;
}


void MyGLWidget::initializeGL()
{
    m_vao.create();
    if (m_vao.isCreated())
        m_vao.bind();
    m_Context = context();

    m_vbo.create();
    qDebug() << m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    /*m_texture = new QOpenGLTexture(QImage(...));

    m_shader = new QOpenGLShader(...);
    m_program = new QOpenGLShaderProgram(...);*/


}

void MyGLWidget::play(QByteArray Payload)
{

    m_vbo.allocate((void *)Payload.constData(),Payload.length());

    show();
    update();
}
