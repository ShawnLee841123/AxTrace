/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_Scene2D.h"
#include "AT4_Message.h"

//--------------------------------------------------------------------------------------------
Scene2D::Scene2D(Begin2DSceneMessage* msg)
	: m_sceneName(msg->getSceneName())
	, m_sceneRect(msg->getSceneRect())
	, m_actorMapIndex(0)
	, m_updating(false)
	, m_updatingRect(msg->getSceneRect())
{
	_parserSceneDefine(msg->getSceneDefine());
}

//--------------------------------------------------------------------------------------------
Scene2D::~Scene2D()
{

}

//--------------------------------------------------------------------------------------------
void Scene2D::beginScene(const Begin2DSceneMessage* msg)
{
	ActorMap& updatingActorsMap = m_actorMap[1-m_actorMapIndex];

	m_updating = true;
	m_updatingRect = msg->getSceneRect();
	updatingActorsMap.clear();
	m_updatingSceneDefine = msg->getSceneDefine();
}

//--------------------------------------------------------------------------------------------
void Scene2D::updateActor(const Update2DActorMessage* msg, const Filter::Actor2DResult& filterResult)
{
	if (!m_updating) return;
	ActorMap& updatingActorsMap = m_actorMap[1 - m_actorMapIndex];

	ActorMap::iterator it = updatingActorsMap.find(msg->getActorID());
	if (it == updatingActorsMap.end()) {
		Actor actor;
		actor.actorID = msg->getActorID();
		actor.pos = msg->getActorPosition();
		actor.dir = msg->getActorDir();
		actor.type = filterResult.type;
		actor.size = filterResult.size;
		actor.fillColor = filterResult.fillColor;
		actor.borderColor = filterResult.borderColor;
		actor.info = msg->getActorInfo();

		updatingActorsMap.insert(msg->getActorID(), actor);
	}
	else
	{
		Actor& actor = *it;

		actor.pos = msg->getActorPosition();
		actor.dir = msg->getActorDir();
		actor.type = filterResult.type;
		actor.size = filterResult.size;
		actor.fillColor = filterResult.fillColor;
		actor.borderColor = filterResult.borderColor;
	}
}

//--------------------------------------------------------------------------------------------
void Scene2D::endScene(const End2DSceneMessage* msg)
{
	if (!m_updating) return;

	m_updating = false;
	m_actorMapIndex = 1 - m_actorMapIndex;
	m_sceneRect = m_updatingRect;

	_parserSceneDefine(m_updatingSceneDefine);
}

//--------------------------------------------------------------------------------------------
void Scene2D::clean(void)
{
	ActorMap& actorsMap = m_actorMap[m_actorMapIndex];

	actorsMap.clear();
}

//--------------------------------------------------------------------------------------------
void Scene2D::walk(Scene2D::ActorWalkFunc walkFunc)
{
	ActorMap& actorsMap = m_actorMap[m_actorMapIndex];

	for (ActorMap::iterator it = actorsMap.begin(); it != actorsMap.end(); ++it)
	{
		const Actor& actor = it.value();
		walkFunc(actor);
	}
}

//--------------------------------------------------------------------------------------------
void Scene2D::_parserSceneDefine(const QJsonObject& sceneInfo)
{
	m_gridDefined = false;

	//parser gridSize
	if (sceneInfo.contains("gridSize") && sceneInfo["gridSize"].isArray()) {
		QJsonArray gridSizeArray = sceneInfo["gridSize"].toArray();
		if (gridSizeArray.size() >= 2) {
			m_gridSize = QSizeF(abs(gridSizeArray[0].toDouble()), abs(gridSizeArray[1].toDouble()));

			m_gridDefined = true;
		}
	}

	//default gridPoint
	m_gridPoint = QPointF(0, 0);

	//parser gridSize
	if (m_gridDefined && sceneInfo.contains("gridPoint") && sceneInfo["gridPoint"].isArray()) {
		QJsonArray gridPointArray = sceneInfo["gridPoint"].toArray();
		if (gridPointArray.size() >= 2) {
			double gridPointX = gridPointArray[0].toDouble();
			double gridPointY = gridPointArray[1].toDouble();

			double absPointX = abs(gridPointX);
			double absPointY = abs(gridPointY);

			int flagX = gridPointX > 0 ? 1 : -1;
			int flagY = gridPointY > 0 ? 1 : -1;
			
			if (absPointX > m_gridSize.width()) {
				gridPointX = flagX*(absPointX - (int)(absPointX / m_gridSize.width()) * m_gridSize.width());
			}
			if (gridPointY > m_gridSize.height()) {
				gridPointY = flagY * (absPointY - (int)(absPointY / m_gridSize.height()) * m_gridSize.height());
			}

			m_gridPoint = QPointF(gridPointX, gridPointY);
		}
	}
}
