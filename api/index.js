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

// Root route
app.get('/', (req, res) => {
  res.json({
    message: 'Todo API is running',
    endpoints: {
      tasks: {
        GET: '/tasks - List all tasks',
        POST: '/tasks - Create a new task',
        PUT: '/tasks/:id - Mark task as completed',
        DELETE: '/tasks/:id - Delete a task'
      }
    },
    version: '1.0.0'
  });
});

// API routes
const router = express.Router();

router.get('/tasks', isAuthorized, async (req, res) => {
  await loadTasks();
  res.json({ tasks });
});

router.post('/tasks', isAuthorized, async (req, res) => {
  if (!req.body || !req.body.description) {
    return res.status(400).json({ error: 'Description is required' });
  }
  
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

router.put('/tasks/:id', isAuthorized, async (req, res) => {
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

router.delete('/tasks/:id', isAuthorized, async (req, res) => {
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

// Mount router
app.use(router);

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