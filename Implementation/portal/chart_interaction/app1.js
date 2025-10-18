// 数据标签（x轴）
const labels = ['APC', 'Tank', 'Jeep', 'Artillery', 'Bike', 'Drone'];

// 初始数据（y轴）
let data = [12, 25, 9, 6, 15, 4];

// 生成随机整数函数
function rand(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

// 创建图表
const ctx = document.getElementById('barChart');
const barChart = new Chart(ctx, {
    type: 'bar',
    data: {
        labels: labels,
        datasets: [{
            label: '使用次数',
            data: data,
            backgroundColor: 'rgba(75,192,192,0.6)',
            borderColor: 'rgba(75,192,192,1)',
            borderWidth: 1
        }]
    },
    options: {
        responsive: false,
        scales: {
            y: { beginAtZero: true }
        },
        onClick: (event, activeEls) => {
            if (activeEls.length === 0) return;  // 没点到柱子直接返回
            const index = activeEls[0].index;    // 获取点到第几个柱子
            const label = barChart.data.labels[index];
            const value = barChart.data.datasets[0].data[index];
            alert(`你点击了 ${label}，使用次数是 ${value}`);   //到这里，先研究整个简单的代码，这个是
        }
    }
});

// 点击按钮 → 随机刷新数据
// 当你点击按钮时，后面 () 里的函数会自动执行。
//这个()=>rand(0,30)的意思就是每一个元素都执行函数，然后取代原来的元素
document.getElementById('refreshBtn').addEventListener('click', () => {
    const newData = labels.map(() => rand(0, 30));
    barChart.data.datasets[0].data = newData;  //看访问的方法，将新的数组，赋值给barChart.data.datasets[0].data
    // 上面是最重要的，就是替换的data：data哪一行
    // 还有就是可以有多个datasets，那么就是[0][1]...
    barChart.update();  //让图表重新渲染（重新绘制）。这个是Chart.js自带的方法
});

//map的使用，对数组里面的“每一个元素执行后面的x*2,并将返回值代替原来的元素”
// const numbers = [1, 2, 3, 4];
// const doubled = numbers.map(x => x * 2);
// console.log(doubled);
