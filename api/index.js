const fs = require('fs').promises;

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
function isAuthorized(req) {
  const auth = req.headers.authorization;
  return auth === 'Basic dXNlcjpwYXNz'; // Same as your C code
}

module.exports = async (req, res) => {
  // CORS headers
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Authorization, Content-Type');

  // Handle OPTIONS request
  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }

  // Check authorization
  if (!isAuthorized(req)) {
    return res.status(401).json({ error: 'Unauthorized access' });
  }

  await loadTasks();

  try {
    switch (req.method) {
      case 'GET':
        return res.status(200).json({ tasks });

      case 'POST':
        const newTask = {
          id: tasks.length + 1,
          description: req.body.description,
          completed: false
        };
        tasks.push(newTask);
        await saveTasks();
        return res.status(201).json(newTask);

      case 'PUT':
        const taskId = parseInt(req.query.id);
        const task = tasks.find(t => t.id === taskId);
        if (task) {
          task.completed = true;
          await saveTasks();
          return res.status(200).json(task);
        }
        return res.status(404).json({ error: 'Task not found' });

      case 'DELETE':
        const deleteId = parseInt(req.query.id);
        const index = tasks.findIndex(t => t.id === deleteId);
        if (index !== -1) {
          tasks.splice(index, 1);
          await saveTasks();
          return res.status(200).json({ deleted: `Task ${deleteId}` });
        }
        return res.status(404).json({ error: 'Task not found' });

      default:
        return res.status(405).json({ error: 'Method not allowed' });
    }
  } catch (error) {
    console.error('Error:', error);
    return res.status(500).json({ error: 'Internal server error' });
  }
}; 