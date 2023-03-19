/**
 * Sort the given array of file names in descending order
 * by the date from the filename.
 * @param {array} array ["2023-03-16.jsonl", ...]
 */
export const sortDesc = (array) => {
  return array.sort(
    (a, b) => new Date(b.split('.')[0]) - new Date(a.split('.')[0]),
  )
}

/**
 * Add leading zero to he given number
 * @param {Number} value
 * @returns {String}
 */
export const leadingZero = (value) => ('0' + value).slice(-2)

export const CHART_DATA_TEMPLATE = {
  labels: [],
  datasets: [
    {
      label: 'Temperature',
      data: [],
      borderColor: '#dc2626',
      borderWidth: 2,
      fill: false,
      cubicInterpolationMode: 'monotone',
      tension: 0.9,
      yAxisID: 'temperature',
    },
    {
      label: 'Humidity',
      data: [],
      borderColor: '#10b981',
      borderWidth: 2,
      fill: false,
      cubicInterpolationMode: 'monotone',
      tension: 0.9,
      yAxisID: 'humidity',
    },
  ],
}

export const getChartOptions = (xAxisTitle, xAxisLabels) => ({
  responsive: true,
  maintainAspectRatio: false,
  scales: {
    x: {
      // type: 'time',
      // time: {
      //   tooltipFormat: 'Date',
      // },
      title: {
        display: true,
        text: xAxisTitle,
      },
      ticks: {
        callback: (value, index) => {
          return index % 3 === 0 ? xAxisLabels[value] : ''
        },
      },
    },
    temperature: {
      title: { display: true, text: 'Temperature, C' },
      type: 'linear',
      display: true,
      position: 'left',
      suggestedMin: 14,
      suggestedMax: 23,
      border: { color: '#dc2626' },
    },
    humidity: {
      title: { display: true, text: 'Humidity, %' },
      type: 'linear',
      display: true,
      position: 'right',
      grid: { drawOnChartArea: false },
      suggestedMin: 36,
      suggestedMax: 47,
      border: { color: '#10b981' },
    },
  },
  interaction: {
    intersect: false,
    mode: 'index',
  },
  plugins: {
    title: { display: true, text: 'Temperature & Humidity' },
    decimation: { enabled: true, algorith: 'min-max' },
    legend: { display: false },
  },
})
