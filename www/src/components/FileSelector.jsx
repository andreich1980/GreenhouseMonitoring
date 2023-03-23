import { Fragment, useEffect, useState } from 'react'
import { Listbox, Transition } from '@headlessui/react'
import { CheckIcon, ChevronUpDownIcon } from '@heroicons/react/20/solid'

const LOADING_STATE = { index: 0, date: 'Loading...' }

export default function FileSelector({
  filesList = [],
  currentIndex,
  onChange,
}) {
  const [selected, setSelected] = useState(
    filesList[currentIndex] ?? LOADING_STATE,
  )

  useEffect(() => {
    const selected = filesList[currentIndex]
    if (selected) {
      setSelected(filesList[currentIndex])
    }
  }, [currentIndex])

  return (
    <Listbox value={selected} onChange={onChange} disabled={!filesList.length}>
      <div className="relative mt-1 w-40 disabled:bg-red-500">
        <Listbox.Button className="relative w-full cursor-default rounded-lg bg-white py-2 pl-3 pr-10 text-left shadow focus:outline-none focus-visible:border-indigo-500 focus-visible:ring-2 focus-visible:ring-white focus-visible:ring-opacity-75 focus-visible:ring-offset-2 focus-visible:ring-offset-emerald-300 disabled:bg-gray-100 dark:bg-gray-700 sm:text-sm">
          <span className="block truncate">{selected.date}</span>
          <span className="pointer-events-none absolute inset-y-0 right-0 flex items-center pr-2">
            <ChevronUpDownIcon
              className="h-5 w-5 text-gray-400"
              aria-hidden="true"
            />
          </span>
        </Listbox.Button>
        <Transition
          as={Fragment}
          leave="transition ease-in duration-100"
          leaveFrom="opacity-100"
          leaveTo="opacity-0"
        >
          <Listbox.Options className="absolute mt-1 max-h-60 w-full overflow-auto rounded-md bg-white py-1 text-base shadow-lg ring-1 ring-black ring-opacity-5 focus:outline-none dark:bg-gray-700 sm:text-sm">
            {filesList.map((file) => (
              <Listbox.Option
                key={file.index}
                value={file}
                className={({ active }) =>
                  `relative cursor-default select-none py-2 pl-10 pr-4 ${
                    active
                      ? 'bg-emerald-100 text-emerald-900 dark:bg-gray-600 dark:text-emerald-500'
                      : 'text-gray-900 dark:text-gray-100'
                  }`
                }
              >
                {({ selected }) => (
                  <>
                    <span
                      className={`block truncate ${
                        selected ? 'font-medium' : 'font-normal'
                      }`}
                    >
                      {file.date}
                    </span>
                    {selected ? (
                      <span className="absolute inset-y-0 left-0 flex items-center pl-3 text-emerald-500">
                        <CheckIcon className="h-5 w-5" aria-hidden="true" />
                      </span>
                    ) : null}
                  </>
                )}
              </Listbox.Option>
            ))}
          </Listbox.Options>
        </Transition>
      </div>
    </Listbox>
  )
}
