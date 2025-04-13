const express = require('express');
const fs = require('fs').promises;

const app = express();
app.use(express.json());

// In-memory storage for tasks (since Vercel functions are stateless)
let tasks = [];

// Load tasks from file if exists
async function loadTasks() {
  try {
    const data = await fs.readFile('/tmp/tasks.json', 'utf8');
    tasks = JSON.parse(data);
  } catch (error) {
    tasks = [];
  }
}

// Save tasks to file
async function saveTasks() {
  try {
    await fs.writeFile('/tmp/tasks.json', JSON.stringify(tasks));
  } catch (error) {
    console.error('Error saving tasks:', error);
  }
}

// Authentication middleware
function isAuthorized(req, res, next) {
  const auth = req.headers.authorization;
  if (auth === 'Basic dXNlcjpwYXNz') {
    next();
  } else {
    res.status(401).json({ error: 'Unauthorized access' });
  }
}

// CORS middleware
app.use((req, res, next) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Authorization, Content-Type');
  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }
  next();
});

// Routes
app.get('/tasks', isAuthorized, async (req, res) => {
  await loadTasks();
  res.json({ tasks });
});

app.post('/tasks', isAuthorized, async (req, res) => {
  await loadTasks();
  const newTask = {
    id: tasks.length + 1,
    description: req.body.description,
    completed: false
  };
  tasks.push(newTask);
  await saveTasks();
  res.status(201).json(newTask);
});

app.put('/tasks/:id', isAuthorized, async (req, res) => {
  await loadTasks();
  const taskId = parseInt(req.params.id);
  const task = tasks.find(t => t.id === taskId);
  if (task) {
    task.completed = true;
    await saveTasks();
    res.json(task);
  } else {
    res.status(404).json({ error: 'Task not found' });
  }
});

app.delete('/tasks/:id', isAuthorized, async (req, res) => {
  await loadTasks();
  const taskId = parseInt(req.params.id);
  const index = tasks.findIndex(t => t.id === taskId);
  if (index !== -1) {
    tasks.splice(index, 1);
    await saveTasks();
    res.json({ deleted: `Task ${taskId}` });
  } else {
    res.status(404).json({ error: 'Task not found' });
  }
});

// Error handler
app.use((err, req, res, next) => {
  console.error(err);
  res.status(500).json({ error: 'Internal server error' });
});

// For local development
if (require.main === module) {
  const port = process.env.PORT || 3000;
  app.listen(port, () => {
    console.log(`Server running on port ${port}`);
  });
}

// For Vercel
module.exports = app; 