#pragma execution_character_set("utf-8") 

#include <QtWidgets>
#include "ImageViewer.h"

ImageViewer::ImageViewer()
	: image_label(new QLabel(this))
	, scrollArea(new QScrollArea(this))
	, status_label(new QLabel(this))
{
	image_label->setBackgroundRole(QPalette::Base);
	image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	image_label->setScaledContents(true);

	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidget(image_label);
	scrollArea->setVisible(false);
	setCentralWidget(scrollArea);
	statusBar()->addPermanentWidget(status_label);

	resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

	createActions();

	set_widget = new QWidget();

	tool_bar = NULL;
	image_index = -1;

	statusBar()->showMessage("Choose source images and Set target path");
}
void ImageViewer::createActions()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("File"));
	QAction *openAct = fileMenu->addAction(tr("Choose imgs"), this, &ImageViewer::open);
	QAction *exitAct = fileMenu->addAction(tr("Exit"), this, &QWidget::close);

	QMenu *toolMenu = menuBar()->addMenu(tr("Tool"));
	QAction *setAct = toolMenu->addAction(tr("Set"), this, &ImageViewer::set);

	QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
	helpMenu->addAction(tr("About"), this, &ImageViewer::about);
	helpMenu->addAction(tr("About Qt"), &QApplication::aboutQt);
}
static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
	static bool firstDialog = true;

	if (firstDialog) {
		firstDialog = false;
		const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}

	QStringList mimeTypeFilters;
	const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
		? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
	foreach(const QByteArray &mimeTypeName, supportedMimeTypes)
		mimeTypeFilters.append(mimeTypeName);
	mimeTypeFilters.sort();
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setMimeTypeFilters(mimeTypeFilters);

	dialog.selectMimeTypeFilter("image/jpeg");
	if (acceptMode == QFileDialog::AcceptSave)
		dialog.setDefaultSuffix("jpg");
}
void ImageViewer::open()
{
	QFileDialog dialog(this, tr("choose images"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
	if (dialog.exec() != QDialog::Accepted) 
		return;
	images_path.clear();
	images_path = dialog.selectedFiles();

	if (images_path.size() == 0)
	{
		QMessageBox::information(this, "error", "One image should be choosed at least.");
		return;
	}

	image_index = 0;
	load_image(images_path[image_index]);
	this->setWindowTitle(QFileInfo(images_path[0]).fileName());
	status_label->setText(QString::number(image_index+1) + "/" + QString::number(images_path.size()));

	if (target_path.size() == 0)
	{
		statusBar()->showMessage("Set target path");
	}
	else
	{
		statusBar()->showMessage("Ready!");
	}
}
void ImageViewer::set()
{
	set_widget->setWindowTitle(tr("set"));
	set_widget->resize(240, 60);

	set_widget->setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint);
	set_widget->setWindowFlags(set_widget->windowFlags() | Qt::WindowStaysOnTopHint);

	QLabel *label = new QLabel(set_widget);
	label->setText(tr("目标文件夹数量："));
	label->setGeometry(30, 20, 130, 20);

	QSpinBox *spinbox = new QSpinBox(set_widget);
	spinbox->setRange(0,12);
	spinbox->setSingleStep(1);
	spinbox->setGeometry(30+label->width(), 20, 50, 20);

	connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_slot(int)));

	saveconfig_button = new QPushButton(set_widget);
	saveconfig_button->setText(tr("save"));
	connect(saveconfig_button, &QPushButton::clicked, this, &ImageViewer::save_config);

	spinbox->setValue(2);
}
void ImageViewer::spinbox_slot(int num)
{
	foreach(auto i, path_edit)
	{
		delete i;
	}
	path_edit.clear();
	foreach(auto i, path_button)
	{
		delete i;
	}
	path_button.clear();

	quint16 margin1 = 10, margin2 = 60;
	quint16 width1 = 180, width2 = 40, height = 20;
	set_widget->resize(240, margin2 + margin1 * (num + 2) + height * (num+1));

	set_widget->close();

	for (qint16 i = 0; i < num; ++i)
	{
		path_edit.push_back(new QLineEdit(set_widget));
		path_button.push_back(new QPushButton(set_widget));
		connect(path_button[i], &QPushButton::clicked, this, &ImageViewer::push_button);
		path_edit[i]->setGeometry(margin1, margin1 + margin2 + i * (height + margin1), width1, height);
		path_button[i]->setGeometry(margin1 + width1, margin1 + margin2 + i * (height + margin1), width2, height);
		path_button[i]->setText(tr("..."));
	}
	saveconfig_button->setGeometry(margin1, margin1 + margin2 + num * (height + margin1), 220, height);

	set_widget->show();
}

void ImageViewer::push_button()
{
	set_widget->hide();
	QPushButton *btn = qobject_cast<QPushButton*>(sender());
	for (quint16 i = 0; i < path_button.size(); ++i)
	{
		if (btn == this->path_button[i])
		{
			path_edit[i]->setText(QFileDialog::getExistingDirectory(this, "choose directory" + QString::number(i), QDir::currentPath()));
		}
	}
	set_widget->show();
}
void ImageViewer::save_config()
{
	set_widget->close();

	target_path.clear();
	foreach(auto i, path_edit)
	{
		target_path << i->text();
	}

	if (tool_bar != NULL)
	{
		foreach(auto i, tool_bar->actions())
		{
			tool_bar->removeAction(i);
		}
		this->removeToolBar(tool_bar);
		tool_bar = NULL;
	}
	tool_bar = addToolBar(tr("Move"));
	for (quint16 i = 0; i < target_path.size(); ++i)
	{
		tool_bar->addAction("path" + QString::number(i), this, &ImageViewer::move);
		tool_bar->actions()[i]->setStatusTip("move current image under the path"+QString::number(i));
	}
	if (images_path.size() == 0)
	{
		statusBar()->showMessage("Choose the images that need be classfied");
	}
	else
	{
		statusBar()->showMessage("Ready!");
	}
}
void ImageViewer::move()
{
	QAction *act = qobject_cast<QAction*>(sender());
	for (quint16 i = 0; i < target_path.size(); ++i)
	{
		if (act->text() == ("path" + QString::number(i)))
		{
			QString filename = QFileInfo(images_path[image_index]).fileName();
			if (!QFile::copy(images_path[image_index], target_path[i] + "/" + filename))
				QMessageBox::information(this, "error", "image copy error");

			image_index++;
			if (image_index >= images_path.size())
			{
				QMessageBox::information(this, "note", "all images are classfied");
				this->close();
			}
			else
			{
				load_image(images_path[image_index]);
				this->setWindowTitle(QFileInfo(images_path[image_index]).fileName());
				status_label->setText(QString::number(image_index + 1) + "/" + QString::number(images_path.size()));
			}
			return;
		}
	}
}
void ImageViewer::load_image(const QString& file_name)
{
	QImageReader reader(file_name);
	reader.setAutoTransform(true);

	const QImage newImage = reader.read();
	image_label->setPixmap(QPixmap::fromImage(newImage));
	image_label->adjustSize();

	scrollArea->setVisible(true);
}
void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
	if (!mouseleft_pressed)
		return;

	scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - (event->pos().x() - mouselast_pos.x()));
	scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - (event->pos().y() - mouselast_pos.y()));
	mouselast_pos = event->pos();
}
void ImageViewer::mousePressEvent(QMouseEvent *event)
{
	mouseleft_pressed = true;
	mouselast_pos = event->pos();
}
void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
	mouseleft_pressed = false;
}
void ImageViewer::about()
{
	QMessageBox::information(this, "note", "Browse https://github.com/iamluoyijie/ImageViewer \nor Email yijieluo233@qq.com if necessary");
}
ImageViewer::~ImageViewer()
{
	delete set_widget;
}