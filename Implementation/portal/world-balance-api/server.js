'use strict';


const express = require('express');
const cors = require('cors');
require('dotenv').config();

const { pool, testConnection } = require('./db');

const app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());


app.get('/', (req, res) => {
  res.send('mysql is running!!!!!');
});



// 1 get all
app.get('/api/gamedatas', async (req, res) => {
  try {
    const [rows] = await pool.query('SELECT * FROM GameData ORDER BY id DESC');
    res.json(rows);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 2 get_by_id
app.get('/api/battlerecords/:id', async (req, res) => {
  try {
    const [rows] = await pool.query('SELECT * FROM battle_records WHERE id = ?', [req.params.id]);
    if (rows.length === 0) return res.status(404).json({ error: 'Record not found' });
    res.json(rows[0]);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 3 add
app.post('/api/battlerecords/add', async (req, res) => {
  try {
    const data = req.body;
    const sql = `
      INSERT INTO battle_records
      (PlayerIP, PlayerName, VehicleID, PlayTime, Win, Lost, Cost, Kills, Deaths, TakeDamage, GetDamage, EndTime, RoundNum)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const values = [
      data.PlayerIP,
      data.PlayerName,
      data.VehicleID,
      data.PlayTime,
      data.Win,
      data.Lost,
      data.Cost,
      data.Kills,
      data.Deaths,
      data.TakeDamage,
      data.GetDamage,
      data.EndTime,  // 'YYYY-MM-DD HH:MM:SS'
      data.RoundNum
    ];
    const [result] = await pool.query(sql, values);
    res.json({ message: 'Battle record added successfully!', id: result.insertId });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 4 update
app.put('/api/gamedatas/update/:id', async (req, res) => {
  try {
    const data = req.body;
    const sql = `
      UPDATE GameData SET
        PlayerIP=?, PlayerName=?, PlayTime=?, Win=?, Lost=?, Cost=?, Kills=?, Deaths=?,
        VehicleID=?, TakeDamage=?, GetDamage=?, EndTime=?, Round=?
      WHERE id=?
    `;
    const values = [
      data.PlayerIP,
      data.PlayerName,
      data.PlayTime,
      data.Win,
      data.Lost,
      data.Cost,
      data.Kills,
      data.Deaths,
      data.VehicleID,
      data.TakeDamage,
      data.GetDamage,
      data.EndTime,
      data.Round,
      req.params.id
    ];
    await pool.query(sql, values);
    res.json({ message: 'GameData updated successfully!' });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 5 delete
app.delete('/api/gamedatas/delete/:id', async (req, res) => {
  try {
    await pool.query('DELETE FROM GameData WHERE id = ?', [req.params.id]);
    res.json({ message: 'deleted successfully!' });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// test
testConnection()
  .then(() => {
    app.listen(port, () => {
      console.log(`GameData API (MySQL) running on http://localhost:${port}`);
    });
  })
  .catch(err => {
    console.error('connection failed:', err.message);
    process.exit(1);
  });

