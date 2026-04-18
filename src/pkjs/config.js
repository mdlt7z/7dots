module.exports = [
  {
    type: "heading",
    defaultValue: "7dots"
  },
  {
    type: "section",
    items: [
      {
        type: "heading",
        defaultValue: "Health"
      },
      {
        type: "slider",
        messageKey: "StepGoal",
        label: "Step Goal",
        defaultValue: 5000,
        min: 100,
        max: 20000,
        step: 100
      }
    ]
  },
  {
    type: "submit",
    defaultValue: "Save Settings"
  },
  {
    type: "text",
    defaultValue:
      "7dots is currently being developed at a relaxed pace :) Please feel free to reach out to the repository."
  },
  {
    type: "text",
    defaultValue: "2026 © mdlt7z"
  }
];
