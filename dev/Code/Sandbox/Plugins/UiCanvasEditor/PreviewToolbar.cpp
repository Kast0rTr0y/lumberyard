/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "stdafx.h"

#include "EditorCommon.h"

PreviewToolbar::PreviewToolbar(EditorWindow* parent)
    : QToolBar("Preview Toolbar", parent)
    , m_viewportSizeLabel(new QLabel(parent))
    , m_canvasScaleLabel(new QLabel(parent))
{
    setObjectName("PreviewToolbar");    // needed to save state
    setFloatable(false);

    // Add the current viewport resolution label
    addSeparator();
    m_viewportSizeLabel->setToolTip(QString("The current size of the viewport"));
    addWidget(m_viewportSizeLabel);

    // Add the combo box to select a canvas size
    addSeparator();
    QLabel* canvasSizeLabel = new QLabel("Preview canvas size: ", parent);
    addWidget(canvasSizeLabel);
    m_canvasSizeToolbarSection.reset(new PreviewCanvasSizeToolbarSection(this, false));

    // Add the canvas scale label
    addSeparator();
    m_canvasScaleLabel->setToolTip(QString("The scale used to fit the canvas in the viewport"));
    addWidget(m_canvasScaleLabel);

    // Add a spacer widget to force the Edit button to be on the right of the toolbar area
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(spacer);

    // Add the Edit button to the right of the toolbar
    m_editButton = new QPushButton("End Preview", parent);
    QObject::connect(m_editButton,
        &QPushButton::clicked,
        [parent](bool checked)
        {
            parent->ToggleEditorMode();
        });

    // Enable Edit button only while in Preview mode
    QObject::connect(parent,
        &EditorWindow::EditorModeChanged,
        [=](UiEditorMode mode)
        {
            m_editButton->setEnabled(mode == UiEditorMode::Preview);
        });
    m_editButton->setEnabled(parent->GetEditorMode() == UiEditorMode::Preview);

    m_editButton->setToolTip(QString("Switch back to Edit mode"));

    // this uses the "primary button" style from the global style sheet
    m_editButton->setProperty("class", "Primary");

    m_editButton->setIcon(QIcon(":/Icons/PreviewStop.png"));

    addWidget(m_editButton);

    parent->addToolBar(this);
}

void PreviewToolbar::ViewportHasResized(QResizeEvent* ev)
{
    m_viewportSizeLabel->setText(QString("Viewport size: %1 x %2").arg(QString::number(ev->size().width()),
            QString::number(ev->size().height())));
}

void PreviewToolbar::UpdatePreviewCanvasScale(float scale)
{
    m_canvasScaleLabel->setText(QString("Canvas scale: %1%").arg(QString::number(scale * 100.0f)));
}

#include <PreviewToolbar.moc>
