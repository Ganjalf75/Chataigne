/*
  ==============================================================================

	ActionUI.cpp
	Created: 28 Oct 2016 8:05:24pm
	Author:  bkupe

  ==============================================================================
*/

#include "ActionUI.h"

#include "Module/ui/ModuleUI.h"
#include "StateMachine/StateManager.h"
#include "Common/Processor/Action/Consequence/Consequence.h"
#include "CustomVariables/CVGroup.h"
#include "../Condition/conditions/StandardCondition/StandardCondition.h"

ActionUI::ActionUI(Action * _action) :
	ProcessorUI(_action),
	action(_action)
{
	acceptedDropTypes.add("Module");
	acceptedDropTypes.add("CommandTemplate");

	action->addAsyncActionListener(this);

	triggerAllUI = action->csmOn->triggerAll->createButtonUI();
	addAndMakeVisible(triggerAllUI);

	progressionUI = action->cdm.validationProgress->createSlider();
	progressionUI->showValue = false;
	addChildComponent(progressionUI);
	progressionUI->setVisible(action->cdm.validationProgress->enabled);
	updateRoleBGColor();
}

ActionUI::~ActionUI()
{
	if (!inspectable.wasObjectDeleted()) action->removeAsyncActionListener(this);
}

void ActionUI::paint(Graphics & g)
{
	BaseItemUI::paint(g);

	if (isDraggingOver)
	{
		g.setColour(BLUE_COLOR.darker());
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);
	}
}

void ActionUI::updateRoleBGColor()
{
	bool isA = action->actionRoles.contains(Action::ACTIVATE);
	bool isD = action->actionRoles.contains(Action::DEACTIVATE);

	if (isA && isD) bgColor = Colours::orange.withSaturation(.4f).darker(1);
	else if (isA) bgColor = GREEN_COLOR.withSaturation(.4f).darker(1);
	else if (isD) bgColor = RED_COLOR.withSaturation(.4f).darker(1);
	else bgColor = ACTION_COLOR.withSaturation(.4f).darker(1);
}

void ActionUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	ProcessorUI::controllableFeedbackUpdateInternal(c);
	if (c == action->cdm.validationTime)
	{
		bool v = action->cdm.validationProgress->enabled;
		if (progressionUI->isVisible() != v)
		{
			progressionUI->setVisible(v);
			resized();
		}
	}
}

void ActionUI::resizedInternalHeader(Rectangle<int>& r)
{
	BaseItemUI::resizedInternalHeader(r);
	//validUI->setBounds(r.removeFromRight(headerHeight));
	//r.removeFromRight(2);
	triggerAllUI->setBounds(r.removeFromRight(60));
	if (progressionUI->isVisible())
	{
		progressionUI->setBounds(r.removeFromRight(40).reduced(2, 6));
	}
}

void ActionUI::paintOverChildren(Graphics & g)
{
	BaseItemUI::paintOverChildren(g);
	if (action->cdm.isValid->boolValue() && action->actionRoles.size() == 0) //no special roles like activate or deactivate
	{
		g.setColour(GREEN_COLOR);
		g.drawRoundedRectangle(getMainBounds().toFloat(), rounderCornerSize, 2);
	}
}

void ActionUI::itemDropped(const SourceDetails & details)
{
	BaseItemUI::itemDropped(details);

	String dataType = details.description.getProperty("dataType", "");
	CommandDefinition * def = nullptr;
	bool isInput = false;
	bool isConsequenceTrue = true;

	if (dataType == "Module")
	{
		ModuleUI * mui = dynamic_cast<ModuleUI *>(details.sourceComponent.get());

		PopupMenu pm;
		ControllableChooserPopupMenu actionInputMenu(&mui->item->valuesCC, true, true, 0);

		PopupMenu actionCommandMenuTrue = mui->item->getCommandMenu(20000, CommandContext::ACTION);
		PopupMenu actionCommandMenuFalse = mui->item->getCommandMenu(30000, CommandContext::ACTION);

		pm.addSubMenu("Input", actionInputMenu);
		pm.addSubMenu("Consequence TRUE", actionCommandMenuTrue);
		pm.addSubMenu("Consequence FALSE", actionCommandMenuFalse);

		int result = pm.show();

		if (result > 0)
		{
			isInput = result < 20000;

			if (isInput)
			{
				StandardCondition * c = dynamic_cast<StandardCondition *>(action->cdm.addItem(action->cdm.factory.create(StandardCondition::getTypeStringStatic())));
				Controllable * target = actionInputMenu.getControllableForResult(result);
				if (c != nullptr) c->sourceTarget->setValueFromTarget(target);
			}
			else //command
			{
				isConsequenceTrue = result > 20000 && result < 30000;
				def = mui->item->getCommandDefinitionForItemID(result - 1 - (isConsequenceTrue?20000:30000));
			}
		}
	}
	else if (dataType == "CommandTemplate")
	{
		PopupMenu pm;
		pm.addItem(1, "Consequence TRUE");
		pm.addItem(2, "Consequence FALSE");

		int result = pm.show();
		if (result > 0)
		{
			isConsequenceTrue = result == 1;

			BaseItemUI<CommandTemplate> * tui = dynamic_cast<BaseItemUI<CommandTemplate> *>(details.sourceComponent.get());
			if (tui != nullptr)
			{
				CommandTemplateManager * ctm = dynamic_cast<CommandTemplateManager *>(tui->item->parentContainer.get());
				if (ctm != nullptr) def = ctm->defManager.getCommandDefinitionFor(ctm->menuName, tui->item->niceName);
			}
		}
	}

	if (!isInput && def != nullptr)
	{
		Consequence * c = isConsequenceTrue ? action->csmOn->addItem() : action->csmOff->addItem();
		if(c != nullptr) c->setCommand(def);
	}
}

void ActionUI::newMessage(const Action::ActionEvent & e)
{
	switch (e.type)
	{
	case Action::ActionEvent::ENABLED_CHANGED:
		break;

	case Action::ActionEvent::ROLE_CHANGED:
		updateRoleBGColor();
		shouldRepaint = true;
		break;

	case Action::ActionEvent::VALIDATION_CHANGED:
		if (action->actionRoles.size() == 0) shouldRepaint = true;
		break;

	}
}
