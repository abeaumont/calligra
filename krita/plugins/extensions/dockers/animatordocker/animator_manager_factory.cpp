/*
 *
 *  Copyright (C) 2011 Torio Mlshi <mlshi@lavabit.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "animator_manager_factory.h"

#include <KGlobal>

#include <iostream>

AnimatorManagerFactory::AnimatorManagerFactory()
{
    m_instances.clear();
}

AnimatorManagerFactory::~AnimatorManagerFactory()
{
}

AnimatorManagerFactory* AnimatorManagerFactory::instance()
{
    K_GLOBAL_STATIC(AnimatorManagerFactory, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

AnimatorManager* AnimatorManagerFactory::getManager(KisImage* image, KisCanvas2* canvas)
{
    if (! m_instances[image])
    {
        m_instances[image] = new AnimatorManager(image);
    }
    
    if (canvas)
        m_instances[image]->setCanvas(canvas);
    else
        m_instances[image]->unsetCanvas();
    
    return m_instances[image];
    
}

void AnimatorManagerFactory::init()
{
}
