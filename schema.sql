CREATE DATABASE IF NOT EXISTS trial;
USE trial;

CREATE TABLE IF NOT EXISTS trial_table (
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    amount DOUBLE NOT NULL DEFAULT 0,
    notes TEXT,
    visit_date DATE,
    visit_time TIME,
    category VARCHAR(64),
    tags VARCHAR(255),
    color VARCHAR(16),
    address TEXT
);
