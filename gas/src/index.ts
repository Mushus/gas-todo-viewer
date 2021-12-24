interface Task {
  title: string;
}

function doGet(e: GoogleAppsScript.Events.DoGet) {
  e.contextPath;
  const taskId = e.parameter["taskId"];
  if (taskId === undefined) {
    return json({ message: "parameter taskId is required" });
  }

  let tasks: GoogleAppsScript.Tasks.Schema.Task[];
  try {
    tasks = getTasks(taskId);
  } catch (e) {
    return json({ message: e });
  }

  return json(presentTasks(tasks));
}

function json(obj: unknown) {
  const out = ContentService.createTextOutput();
  out.setMimeType(ContentService.MimeType.JSON);
  out.setContent(JSON.stringify(obj));
  return out;
}

function getTasks(taskId: string) {
  if (!Tasks.Tasks) {
    throw new Error("tasks api not found");
  }

  const tasks = Tasks.Tasks?.list(taskId);
  if (!tasks.items) {
    throw new Error("task not found");
  }

  return tasks.items;
}

function presentTasks(tasks: GoogleAppsScript.Tasks.Schema.Task[]): Task[] {
  return tasks.map(presentTask);
}

function presentTask(task: GoogleAppsScript.Tasks.Schema.Task): Task {
  return {
    title: task.title ?? "",
  };
}

function getTasklist() {
  console.log(Tasks.Tasklists?.list().items);
}
