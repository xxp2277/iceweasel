/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

// React & Redux
const {
  Component,
  createFactory,
} = require("devtools/client/shared/vendor/react");

const PropTypes = require("devtools/client/shared/vendor/react-prop-types");
const { connect } = require("devtools/client/shared/vendor/react-redux");

const frameworkActions = require("devtools/client/framework/actions/index");
const webconsoleActions = require("devtools/client/webconsole/actions/index");

const { l10n } = require("devtools/client/webconsole/utils/messages");
const targetSelectors = require("devtools/client/framework/reducers/targets");

loader.lazyGetter(this, "TARGET_TYPES", function() {
  return require("devtools/shared/resources/target-list").TargetList.TYPES;
});

// Additional Components
const MenuButton = createFactory(
  require("devtools/client/shared/components/menu/MenuButton")
);

loader.lazyGetter(this, "MenuItem", function() {
  return createFactory(
    require("devtools/client/shared/components/menu/MenuItem")
  );
});

loader.lazyGetter(this, "MenuList", function() {
  return createFactory(
    require("devtools/client/shared/components/menu/MenuList")
  );
});

class EvaluationContextSelector extends Component {
  static get propTypes() {
    return {
      selectTarget: PropTypes.func.isRequired,
      updateInstantEvaluationResultForCurrentExpression:
        PropTypes.func.isRequired,
      selectedTarget: PropTypes.object,
      targets: PropTypes.array,
      webConsoleUI: PropTypes.object.isRequired,
    };
  }

  componentDidUpdate(prevProps) {
    if (this.props.selectedTarget !== prevProps.selectedTarget) {
      this.props.updateInstantEvaluationResultForCurrentExpression();
    }
  }

  getIcon(target) {
    if (target.type === TARGET_TYPES.FRAME) {
      return "resource://devtools/client/debugger/images/globe-small.svg";
    }

    if (target.type === TARGET_TYPES.WORKER) {
      return "resource://devtools/client/debugger/images/worker.svg";
    }

    if (target.type === TARGET_TYPES.PROCESS) {
      return "resource://devtools/client/debugger/images/window.svg";
    }

    return null;
  }

  renderMenuItem(target) {
    const { selectTarget, selectedTarget } = this.props;

    const label = target.isMainTarget
      ? l10n.getStr("webconsole.input.selector.top")
      : target.name;

    return MenuItem({
      key: `webconsole-evaluation-selector-item-${target.actorID}`,
      className: "menu-item webconsole-evaluation-selector-item",
      type: "checkbox",
      checked: selectedTarget ? selectedTarget == target : target.isMainTarget,
      label,
      tooltip: target.url,
      icon: this.getIcon(target),
      onClick: () => selectTarget(target.actorID),
    });
  }

  renderMenuItems() {
    const { targets } = this.props;

    // Let's sort the targets (using "numeric" so Content processes are ordered by PID).
    const collator = new Intl.Collator("en", { numeric: true });
    targets.sort((a, b) => collator.compare(a.name, b.name));

    let mainTarget;
    const frames = [];
    const contentProcesses = [];
    const workers = [];

    const dict = {
      [TARGET_TYPES.FRAME]: frames,
      [TARGET_TYPES.PROCESS]: contentProcesses,
      [TARGET_TYPES.WORKER]: workers,
    };

    for (const target of targets) {
      const menuItem = this.renderMenuItem(target);

      if (target.isMainTarget) {
        mainTarget = menuItem;
      } else {
        dict[target.type].push(menuItem);
      }
    }

    const items = [mainTarget];

    if (frames.length > 0) {
      items.push(
        MenuItem({ role: "menuseparator", key: "frames-separator" }),
        ...frames
      );
    }

    if (contentProcesses.length > 0) {
      items.push(
        MenuItem({ role: "menuseparator", key: "process-separator" }),
        ...contentProcesses
      );
    }

    if (workers.length > 0) {
      items.push(
        MenuItem({ role: "menuseparator", key: "worker-separator" }),
        ...workers
      );
    }

    return MenuList(
      { id: "webconsole-console-evaluation-context-selector-menu-list" },
      items
    );
  }

  getLabel() {
    const { selectedTarget } = this.props;

    if (!selectedTarget || selectedTarget.isMainTarget) {
      return l10n.getStr("webconsole.input.selector.top");
    }

    return selectedTarget.name;
  }

  render() {
    const { webConsoleUI, targets, selectedTarget } = this.props;
    const doc = webConsoleUI.document;
    const { toolbox } = webConsoleUI.wrapper;

    if (targets.length <= 1) {
      return null;
    }

    return MenuButton(
      {
        menuId: "webconsole-input-evaluationsButton",
        toolboxDoc: toolbox ? toolbox.doc : doc,
        label: this.getLabel(),
        className:
          "webconsole-evaluation-selector-button devtools-button devtools-dropdown-button" +
          (selectedTarget && !selectedTarget.isMainTarget
            ? " webconsole-evaluation-selector-button-non-top"
            : ""),
        title: l10n.getStr("webconsole.input.selector.tooltip"),
      },
      // We pass the children in a function so we don't require the MenuItem and MenuList
      // components until we need to display them (i.e. when the button is clicked).
      () => this.renderMenuItems()
    );
  }
}

const toolboxConnected = connect(
  state => ({
    targets: targetSelectors.getToolboxTargets(state),
    selectedTarget: targetSelectors.getSelectedTarget(state),
  }),
  dispatch => ({
    selectTarget: actorID => dispatch(frameworkActions.selectTarget(actorID)),
  }),
  undefined,
  { storeKey: "toolbox-store" }
)(EvaluationContextSelector);

module.exports = connect(
  state => state,
  dispatch => ({
    updateInstantEvaluationResultForCurrentExpression: () =>
      dispatch(
        webconsoleActions.updateInstantEvaluationResultForCurrentExpression()
      ),
  })
)(toolboxConnected);
