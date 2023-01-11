#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoSurface>

namespace Ui {
class MyGLWidget;
}

class MyGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit MyGLWidget(QWidget *parent = nullptr);
    ~MyGLWidget();
    void play(QByteArray Payload);

private:
    Ui::MyGLWidget *ui;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLShaderProgram *m_program;
    QOpenGLShader *m_shader;
    QOpenGLTexture *m_texture;
    QOpenGLContext *m_Context;
    QImage img;
    QVideoSurfaceFormat format;

protected:

    void initializeGL()override;
};

#endif // MYGLWIDGET_H
